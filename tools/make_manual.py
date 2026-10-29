#!/usr/bin/env python3
"""Builds manual.html, the user manual, from tools/manual.src.html: the same
page with each picture written as {{IMG:file}}, a file in tools/img/ (a
screenshot from the simulator: `make sim`) -- put into the page as it's
built, so the manual is one file that needs nothing else.

    python3 tools/make_manual.py
"""
import base64
import os
import re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOLS = os.path.join(ROOT, 'tools')


def embed(m):
    path = os.path.join(TOOLS, 'img', m.group(1))
    return 'data:image/png;base64,' + base64.b64encode(open(path, 'rb').read()).decode()


src = open(os.path.join(TOOLS, 'manual.src.html')).read()
page = re.sub(r'\{\{IMG:([^}]+)\}\}', embed, src)
out = os.path.join(ROOT, 'manual.html')
open(out, 'w').write(page)
print(out, len(page), 'bytes')
