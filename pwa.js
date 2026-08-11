(()=>{
const USER='https://ngpfglowsaxmsfdryuev.supabase.co/functions/v1/dropradar-user';
const DATA='https://ngpfglowsaxmsfdryuev.supabase.co/functions/v1/dropradar-api';
let installPrompt=null;
const $=s=>document.querySelector(s);
function text(ru,de,en){const l=(localStorage.getItem('dropradar_lang')||document.documentElement.lang||navigator.language||'en').toLowerCase();return l.startsWith('ru')?ru:l.startsWith('de')?de:en}
function toast(message){let e=$('#pwaToast');if(!e){e=document.createElement('div');e.id='pwaToast';e.style.cssText='position:fixed;z-index:10000;right:16px;bottom:128px;max-width:320px;background:#121719;border:1px solid #354044;color:#e5eaec;border-radius:12px;padding:12px 14px;font:12px/1.4 system-ui;box-shadow:0 10px 30px #0008';document.body.appendChild(e)}e.textContent=message;e.hidden=false;clearTimeout(window.__dropToast);window.__dropToast=setTimeout(()=>e.hidden=true,3500)}
function addButton(id,label,bottom){let b=document.getElementById(id);if(b)return b;b=document.createElement('button');b.id=id;b.textContent=label;b.type='button';b.style.cssText=`position:fixed;z-index:9999;right:16px;bottom:${bottom}px;width:46px;height:46px;border-radius:50%;border:1px solid #364044;background:#111416;color:#dce2e4;box-shadow:0 10px 30px #0008;display:grid;place-items:center;font:19px system-ui;cursor:pointer`;document.body.appendChild(b);return b}
const pushBtn=addButton('pwaPush','🔔',16),installBtn=addButton('pwaInstall','＋',70);installBtn.style.display='none';
function bytes(v){const pad='='.repeat((4-v.length%4)%4),raw=atob((v+pad).replace(/-/g,'+').replace(/_/g,'/')),out=new Uint8Array(raw.length);for(let i=0;i<raw.length;i++)out[i]=raw.charCodeAt(i);return out}
function cloudKey(){let k=localStorage.getItem('dropradar_cloud_key');if(k)return k;const b=new Uint8Array(32);crypto.getRandomValues(b);let s='';b.forEach(x=>s+=String.fromCharCode(x));k=btoa(s).replace(/\+/g,'-').replace(/\//g,'_').replace(/=+$/,'');localStorage.setItem('dropradar_cloud_key',k);return k}
function prefs(){try{return JSON.parse(localStorage.getItem('dropradar_preferences')||'{}')}catch{return{}}}
async function sw(){if(!('serviceWorker'in navigator))throw Error('no service worker');await navigator.serviceWorker.register('./sw.js');return navigator.serviceWorker.ready}
function updateState(){const on=localStorage.getItem('dropradar_alerts')==='1'&&'Notification'in window&&Notification.permission==='granted';pushBtn.style.background=on?'#c9ff32':'#111416';pushBtn.style.color=on?'#090b0c':'#dce2e4';pushBtn.style.borderColor=on?'#c9ff32':'#364044';pushBtn.title=on?text('Уведомления включены','Benachrichtigungen aktiv','Alerts enabled'):text('Включить уведомления','Benachrichtigungen aktivieren','Enable alerts')}
async function togglePush(){
 if(!('serviceWorker'in navigator)||!('PushManager'in window)||!('Notification'in window)){toast(text('Push не поддерживается','Push wird nicht unterstützt','Push is not supported'));return}
 const ios=/iphone|ipad|ipod/i.test(navigator.userAgent),standalone=matchMedia('(display-mode: standalone)').matches||navigator.standalone===true;
 if(ios&&!standalone){toast(text('Сначала добавь DropRadar на экран «Домой», открой его оттуда и затем включи уведомления.','Füge DropRadar zuerst zum Home-Bildschirm hinzu, öffne es dort und aktiviere dann Benachrichtigungen.','Add DropRadar to your Home Screen first, open it from there, then enable alerts.'));return}
 try{
  const reg=await sw(),existing=await reg.pushManager.getSubscription();
  if(existing&&localStorage.getItem('dropradar_alerts')==='1'){
   await Promise.allSettled([fetch(USER+'?action=unsubscribe',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({endpoint:existing.endpoint})}),fetch(DATA+'?action=unsubscribe',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({endpoint:existing.endpoint})})]);
   await existing.unsubscribe();localStorage.setItem('dropradar_alerts','0');updateState();toast(text('Уведомления выключены','Benachrichtigungen deaktiviert','Alerts disabled'));return;
  }
  const permission=await Notification.requestPermission();if(permission!=='granted'){toast(text('Браузер не разрешил уведомления','Browser hat Benachrichtigungen blockiert','Browser did not allow notifications'));return}
  const cfg=await fetch(USER+'?action=config',{cache:'no-store'}).then(r=>r.json());if(!cfg?.vapidPublicKey)throw Error('VAPID unavailable');
  const sub=existing||await reg.pushManager.subscribe({userVisibleOnly:true,applicationServerKey:bytes(cfg.vapidPublicKey)});
  const lang=(localStorage.getItem('dropradar_lang')||'en').slice(0,2);
  const r=await fetch(USER+'?action=subscribe',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({subscription:sub.toJSON(),lang,cloudKey:cloudKey(),preferences:prefs(),silent:false})}),d=await r.json();
  if(!r.ok||!d?.ok)throw Error(d?.error||'subscribe failed');
  localStorage.setItem('dropradar_alerts','1');updateState();toast(text('Персональные уведомления включены','Persönliche Benachrichtigungen aktiv','Personal alerts enabled'));
 }catch(e){console.error(e);toast(text('Не удалось включить push','Push konnte nicht aktiviert werden','Could not enable push'))}
}
pushBtn.addEventListener('click',togglePush);
window.addEventListener('beforeinstallprompt',e=>{e.preventDefault();installPrompt=e;installBtn.style.display='grid'});
installBtn.addEventListener('click',async()=>{if(!installPrompt)return;installPrompt.prompt();await installPrompt.userChoice;installPrompt=null;installBtn.style.display='none'});
window.addEventListener('appinstalled',()=>{installBtn.style.display='none';toast(text('DropRadar установлен','DropRadar installiert','DropRadar installed'))});
sw().catch(()=>{});updateState();
})();
