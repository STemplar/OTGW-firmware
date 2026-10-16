#!/usr/bin/env python3
"""Extract setting, status and fault-code tables from a Remeha/BDR boiler manual (PDF).

Usage: python3 tools/boiler-manual-extract.py MANUAL.pdf OUT.json [--id ID] [--name NAME]

Recognised table rows (pdfplumber table extraction):
  settings   [code, description, default, min, max, level]             (CU-GH16 style)
             [code, description, range, (submenu,) default per variant ...] (CU-GH08/09 style;
             variant names come from the table header row)
  readonly   [code, description, value]          code AM/GM/... in a read-only table
  counters   [code, level, description]          level Gebruiker/Installateur
  codes      [code, description, cause]          code A.00.00 / A00.00
             [group, .specific, description, cause]  group A.00 + specific .42
             [code, display text, description, solution]
  status     rows [text, number] after a STATUS / SUBSTATUS header

Every row records the nearest preceding table caption ("Tab.NN ..."), so rows from e.g.
gas-type adaptation tables stay distinguishable from the factory-settings table.

The output is a draft: it records what the manual says. It does not map anything to
OpenTherm (TSP index, ID5, ID115); that needs a confirmed readout of a real boiler.
Soft hyphens are joined; real hyphens at line ends are kept. Check a page by eye
against the PDF before relying on the result.
"""
import argparse
import collections
import json
import re
import sys

import pdfplumber

O = collections.OrderedDict
CODE = re.compile(r'[A-Z]{2}\d{3}')
FAULT = re.compile(r'[AHE]\.?\d{2}\.\d{2}')
GROUP = re.compile(r'[AHE]\.\d{2}')
SPECIFIC = re.compile(r'\.\d{2}')
LEVELS = {'gebruiker', 'installateur', 'fabrikant'}
TYPES = {'A': 'Waarschuwing', 'H': 'Tijdelijke storing / blokkering', 'E': 'Permanente storing / vergrendeling (reset vereist)'}
CAPTION = re.compile(r'^\s*(Tab\.\s?\d+)\b\s*(.*)$')
DOC = re.compile(r'(\d{6,7})\s*-\s*(v?\.?\s*[0-9A-Za-z]+)\s*-\s*(\d{6,8})')


def clean(cell):
    text = (cell or '').replace('­\n', '').replace('­', '').replace('-\n', '-').replace('\n', ' ')
    return re.sub(r'\s+', ' ', text).strip()


def fault_code(code):
    """Normalise A00.34 and A.00.34 to A.00.34."""
    return code if code[1] == '.' else code[0] + '.' + code[1:]


def split_stacked(table):
    """Rows where several codes share one cell (e.g. 'CP080\nCP081...') become one row per code.
    Columns with the same number of lines are split along; other columns are repeated."""
    for row in table:
        first = (row[0] or '').split('\n') if row else []
        if len(first) > 1 and all(CODE.fullmatch(x.strip()) for x in first):
            n = len(first)
            for i in range(n):
                yield [((cell or '').split('\n')[i] if (cell or '').count('\n') == n - 1 else cell) for cell in row]
        else:
            yield row


def extract(path):
    out = O(settings=[], readonly=[], counters=[], codes=[], status=O(), substatus=O())
    footers = collections.Counter()
    caption = ''
    header = []      # last table header row; continuation tables on the next page reuse it
    previous = None  # last settings entry, for rows that only repeat the code (e.g. CP081 under CP080)
    with pdfplumber.open(path) as pdf:
        for index, page in enumerate(pdf.pages):
            number = index + 1
            text = page.extract_text() or ''
            for match in DOC.finditer(text):
                footers[(match.group(1), re.sub(r'\s+', '', match.group(2)), match.group(3))] += 1
            captions = []
            lines = page.extract_text_lines()
            for position, line in enumerate(lines):
                found = CAPTION.match(line['text'])
                if found:
                    title = found.group(2).strip()
                    if not title and position + 1 < len(lines):  # title printed on the next line
                        title = lines[position + 1]['text'].strip(' -')
                    captions.append((line['top'], re.sub(r'\s+', ' ', found.group(1) + ' ' + title).strip()))
            for found_table in page.find_tables():
                above = [text for top, text in captions if top < found_table.bbox[1]]
                if above:
                    caption = above[-1]
                table = found_table.extract()
                section = None
                group = None
                for row in split_stacked(table):
                    c = [clean(x) for x in row]
                    if not c or not any(c):
                        continue
                    head = c[0].upper()
                    if head in ('CODE', 'NAAM', 'PARAMETER', 'INSTELLING'):
                        header = c
                        continue
                    if head in ('STATUS', 'SUBSTATUS'):
                        section = head.lower()
                        continue
                    if section and len(c) >= 2 and re.fullmatch(r'\d{1,3}', c[-1] or '') and c[0]:
                        out[section][c[-1]] = c[0]
                        continue
                    if CODE.fullmatch(c[0]):
                        if len(c) == 6 and c[5].lower() in LEVELS:
                            out['settings'].append(O([('code', c[0]), ('label', c[1]), ('default', c[2]),
                                                      ('min', c[3]), ('max', c[4]), ('level', c[5]), ('table', caption), ('page', number)]))
                        elif len(c) == 3 and c[1].lower() in LEVELS:
                            out['counters'].append(O([('code', c[0]), ('level', c[1]), ('label', c[2]), ('table', caption), ('page', number)]))
                        elif len(c) >= 4 and len(header) == len(c) and header[2].lower().startswith(('standaard', 'fabriek')):
                            # [code, description, default, other columns ...] (e.g. CO2 adjustment table)
                            entry = O([('code', c[0]), ('label', c[1]), ('default', c[2])])
                            extra = [x for x in c[3:] if x]
                            if extra:
                                entry['extra'] = extra
                            entry['table'] = caption
                            entry['page'] = number
                            out['settings'].append(entry)
                            previous = entry
                        elif len(c) >= 4:
                            # [code, description, range, (submenu,) value per variant ...]
                            names = header[3:] if len(header) == len(c) else []
                            values = c[3:]
                            entry = O([('code', c[0])])
                            same = previous and not c[1] and not c[2]
                            entry['label'] = c[1] or (previous['label'] if same else '')
                            if c[2] or same and previous.get('range'):
                                entry['range'] = c[2] or previous['range']
                            if names and names[0].lower().startswith('submenu'):
                                entry['submenu'] = values[0] or (previous.get('submenu', '') if same else '')
                                names, values = names[1:], values[1:]
                            if len(set(values)) == 1:
                                entry['default'] = values[0]
                            elif names and all(names):
                                entry['default'] = O(zip(names, values))
                            else:
                                entry['default'] = values
                            entry['table'] = caption
                            entry['page'] = number
                            out['settings'].append(entry)
                            previous = entry
                        elif len(c) in (2, 3):
                            entry = O([('code', c[0]), ('label', c[1])])
                            if len(c) == 3 and c[2]:
                                entry['value'] = c[2]
                            entry['table'] = caption
                            entry['page'] = number
                            out['readonly'].append(entry)
                        continue
                    if len(c) == 3 and FAULT.fullmatch(c[0]):
                        code = fault_code(c[0])
                        out['codes'].append(O([('code', code), ('type', TYPES[code[0]]), ('description', c[1]),
                                               ('cause_solution', c[2]), ('table', caption), ('page', number)]))
                        continue
                    if len(c) == 4 and FAULT.fullmatch(c[0]):
                        # [code, display text, description, solution]
                        code = fault_code(c[0])
                        out['codes'].append(O([('code', code), ('type', TYPES[code[0]]), ('display', c[1]),
                                               ('description', c[2]), ('cause_solution', c[3]),
                                               ('table', caption), ('page', number)]))
                        continue
                    if len(c) == 4:
                        if GROUP.fullmatch(c[0]):
                            group = c[0]
                        if group and SPECIFIC.fullmatch(c[1]) and (c[0] in ('', group)):
                            code = group + c[1]
                            out['codes'].append(O([('code', code), ('type', TYPES[code[0]]), ('description', c[2]),
                                                   ('cause_solution', c[3]), ('table', caption), ('page', number)]))
    doc = footers.most_common(1)[0][0] if footers else None
    return out, doc


def main():
    parser = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    parser.add_argument('pdf')
    parser.add_argument('out')
    parser.add_argument('--id', default='')
    parser.add_argument('--name', default='')
    args = parser.parse_args()
    tables, doc = extract(args.pdf)
    result = O()
    result['id'] = args.id
    result['name'] = args.name
    result['language'] = 'nl'
    result['status_of_draft'] = ('CONCEPT — niet in gebruik. Zonder bevestigde herkenning (member-ID, ID127) '
                                 'staat dit profiel niet in kBoilerModelProfiles.')
    if doc:
        result['document'] = O([('number', doc[0]), ('version', doc[1]), ('date', doc[2])])
    result['notes'] = O([
        ('settings', 'Instellingscodes zijn geen OpenTherm-TSP-indexen; de handleiding geeft geen koppeling.'),
        ('codes', 'A = waarschuwing, H = tijdelijke storing/blokkering, E = permanente storing/vergrendeling. '
                  'Geen koppeling met ID5 (OEM fault code) in de handleiding.'),
        ('status', 'Status/substatus zijn displaywaarden. Of ID115 dezelfde codering heeft, is niet bevestigd.')])
    for key in ('status', 'substatus'):
        if tables[key]:
            result[key if key == 'status' else 'substatus_any'] = O(sorted(tables[key].items(), key=lambda kv: int(kv[0])))
    for key in ('settings', 'readonly', 'counters', 'codes'):
        if tables[key]:
            result[key] = tables[key]
    with open(args.out, 'w') as handle:
        handle.write(json.dumps(result, ensure_ascii=False, indent=2) + '\n')
    counts = {k: len(result.get(k if k != 'substatus' else 'substatus_any', [])) for k in
              ('settings', 'readonly', 'counters', 'codes', 'status', 'substatus')}
    print(json.dumps(O([('document', result.get('document')), ('counts', counts)]), ensure_ascii=False))


if __name__ == '__main__':
    sys.exit(main())
