#!/bin/sh

kikit panelize \
    --layout 'grid; rows: 3; cols: 2; space: 2mm' \
    --tabs 'fixed; width: 3mm; vcount: 3; hcount: 3' \
    --cuts 'mousebites; drill: 0.5mm; spacing: 1mm; offset: 0.2mm; prolong: 0.5mm' \
    --post 'millradius: 1mm' \
    edid_nuzzler.kicad_pcb panel.kicad_pcb
