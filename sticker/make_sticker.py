"""Builds sticker.svg: QR code to the hanukkiah's control page (60 x 80 mm).
Run: python make_sticker.py   (needs: pip install segno)"""
import segno

URL = "https://amir684.github.io/Hanukkiah-Galei-HaAliyah/#control"
qr = segno.make(URL, error='q', micro=False)
m = qr.matrix
n = len(m)
W, H = 60, 80                  # mm
qs = 41.0                      # QR size, mm
pad = 3.2                      # white quiet zone, mm
cell = qs / n
ox, oy = (W - qs) / 2, 20.5
d = "".join(f"M{ox+x*cell:.3f} {oy+y*cell:.3f}h{cell:.3f}v{cell:.3f}h-{cell:.3f}z"
            for y, row in enumerate(m) for x, v in enumerate(row) if v)
cx, cy = W / 2, oy + qs / 2
by = oy + qs + pad
svg = f'''<svg xmlns="http://www.w3.org/2000/svg" width="{W}mm" height="{H}mm" viewBox="0 0 {W} {H}">
<defs><linearGradient id="bg" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#0a1226"/><stop offset="1" stop-color="#13306a"/></linearGradient>
<linearGradient id="gd" x1="0" x2="1"><stop offset="0" stop-color="#ffd88a"/><stop offset=".5" stop-color="#ffb347"/><stop offset="1" stop-color="#e08a1e"/></linearGradient></defs>
<rect width="{W}" height="{H}" rx="5" fill="url(#bg)"/>
<rect x="1.2" y="1.2" width="{W-2.4}" height="{H-2.4}" rx="4" fill="none" stroke="url(#gd)" stroke-width=".5"/>
<text x="{W/2}" y="11.5" text-anchor="middle" font-family="Frank Ruhl Libre, David, Times New Roman, serif" font-weight="900" font-size="8.6" fill="url(#gd)" direction="rtl">גלי העלייה</text>
<text x="{W/2}" y="15.8" text-anchor="middle" font-family="Arial, sans-serif" font-size="2.9" fill="#9fd8ff" direction="rtl">סרקו לשליטה בחנוכייה</text>
<rect x="{ox-pad}" y="{oy-pad}" width="{qs+2*pad}" height="{qs+2*pad}" rx="2.5" fill="#fff"/>
<path d="{d}" fill="#0a1226"/>
<g transform="translate({cx} {cy})"><circle r="4.6" fill="#fff"/><circle r="3.9" fill="url(#gd)"/>
<path d="M0 2.3 C-1.7 1.7 -1.6 -0.9 0 -3 C0.45 -1.6 1.7 -0.8 1.5 0.6 C1.4 1.6 0.8 2.3 0 2.3Z" fill="#fff8e6"/></g>
<path d="M4 {by+3.6} q4 -2.4 8 0 t8 0 t8 0 t8 0 t8 0 t8 0 t8 0" fill="none" stroke="#3aa8ff" stroke-width=".6" opacity=".8"/>
<text x="{W/2}" y="{by+8.6}" text-anchor="middle" font-family="Arial, sans-serif" font-weight="bold" font-size="3.4" fill="#eae6da" direction="rtl">תכננה ובנתה: נועה יהוד</text>
</svg>'''
open('sticker.svg', 'w', encoding='utf-8').write(svg)
print(f"{n}x{n} modules, {cell:.2f} mm each -> sticker.svg")
