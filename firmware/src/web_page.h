#pragma once
#include <pgmspace.h>

// Phone control page (Hebrew, RTL). Served from flash.
static const char PAGE_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="he" dir="rtl"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>חנוכיית גלי העלייה</title>
<style>
:root{--bg:#141820;--card:#1e2430;--line:#2e3646;--txt:#e8e4d8;--mut:#9aa3b2;--sea:#4fb3ff;--gold:#ffb347}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--txt);font-family:system-ui,Arial,sans-serif;padding:16px;max-width:520px;margin:auto}
h1{font-size:22px;font-weight:600;margin:4px 0 2px;color:var(--gold)}.sub{color:var(--mut);font-size:14px;margin-bottom:14px}
.card{background:var(--card);border:1px solid var(--line);border-radius:14px;padding:14px;margin-bottom:12px}
.card h2{font-size:15px;margin:0 0 10px;color:var(--mut);font-weight:500}
.row{display:flex;gap:8px;flex-wrap:wrap}
button{flex:1;min-width:70px;background:#28303f;color:var(--txt);border:1px solid var(--line);border-radius:10px;padding:12px 6px;font-size:15px}
button.on{background:var(--sea);color:#06121f;border-color:var(--sea);font-weight:600}
#light{width:100%;background:var(--gold);color:#2a1600;font-size:20px;font-weight:700;padding:16px;border:0}
.nights{display:grid;grid-template-columns:repeat(8,1fr);gap:6px;direction:ltr}
.nights button{min-width:0;padding:12px 0}
input[type=range]{width:100%}select,input[type=text],input[type=password]{width:100%;padding:10px;border-radius:8px;border:1px solid var(--line);background:#141820;color:var(--txt);font-size:15px;margin-bottom:8px}
.st{display:grid;grid-template-columns:auto 1fr;gap:4px 12px;font-size:14px}.st span:nth-child(odd){color:var(--mut)}
</style></head><body>
<h1>חנוכיית גלי העלייה</h1>
<div class="sub">ושבו בנים לגבולם</div>

<div class="card"><button id="light" onclick="api('/api/light')">🔥 הדלקה!</button></div>

<div class="card"><h2>מצב</h2><div class="row">
<button data-mode="auto">אוטומטי</button><button data-mode="manual">ידני</button>
<button data-mode="show">כל הנרות</button><button data-mode="demo">הדגמה</button></div></div>

<div class="card"><h2>לילה (ימין = נר ראשון)</h2><div class="nights" id="nights"></div></div>

<div class="card"><h2>בהירות</h2><input type="range" id="bri" min="10" max="255"
 onchange="api('/api/set?brightness='+this.value)"></div>

<div class="card"><h2>צבעים</h2><select id="theme" onchange="api('/api/set?theme='+this.value)">
<option value="0">ים וזהב</option><option value="1">כחול לבן</option>
<option value="2">זהב חם</option><option value="3">קשת</option></select></div>

<div class="card"><h2>סטטוס</h2><div class="st" id="st"></div></div>

<div class="card"><h2>חיבור לרשת WiFi (לשעון מדויק)</h2>
<form method="post" action="/api/wifi"><input type="text" name="ssid" placeholder="שם הרשת">
<input type="password" name="pass" placeholder="סיסמה"><button type="submit">שמור והפעל מחדש</button></form>
<div class="sub" style="margin-top:8px">בלי אינטרנט, השעון מכוון אוטומטית מהטלפון כשנכנסים לדף.</div></div>

<script>
const MODES={auto:'אוטומטי',manual:'ידני',show:'כל הנרות',demo:'הדגמה'};
const nb=document.getElementById('nights');
for(let n=8;n>=1;n--){const b=document.createElement('button');b.textContent=n;b.dataset.n=n;
 b.onclick=()=>api('/api/set?night='+n);nb.appendChild(b);}
document.querySelectorAll('[data-mode]').forEach(b=>b.onclick=()=>api('/api/set?mode='+b.dataset.mode));
let synced=false;
function show(s){
 document.querySelectorAll('[data-mode]').forEach(b=>b.classList.toggle('on',b.dataset.mode==s.mode));
 document.querySelectorAll('#nights button').forEach(b=>b.classList.toggle('on',+b.dataset.n==s.night));
 if(document.activeElement.id!='bri')document.getElementById('bri').value=s.brightness;
 document.getElementById('theme').value=s.theme;
 const rows=[['מצב',MODES[s.mode]],['לילה מוצג',s.night||'-'],
  ['שעה',s.timeValid?s.time:'לא מכוונת'],
  ['חנוכה עכשיו',s.hanukkahNight?('לילה '+s.hanukkahNight):'לא'],
  ['הדלקת נר ראשון',s.nextEve||'-'],
  ['WiFi',s.sta?(s.ssid+' · '+s.ip):'רק רשת החנוכייה']];
 document.getElementById('st').innerHTML=rows.map(r=>'<span>'+r[0]+'</span><span>'+r[1]+'</span>').join('');
 if(!s.timeValid&&!synced){synced=true;api('/api/time?epoch='+Math.floor(Date.now()/1000));}
}
function api(u){return fetch(u).then(r=>r.json()).then(show).catch(()=>{});}
api('/api/state');setInterval(()=>api('/api/state'),3000);
</script></body></html>)HTML";
