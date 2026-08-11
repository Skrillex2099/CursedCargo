const CACHE='dropradar-v071';
const BASE=new URL('./',self.location.href);
const HOME=new URL('dropradar.html',BASE).href;
const CORE=[HOME,new URL('manifest.json',BASE).href,new URL('icon.svg',BASE).href];
self.addEventListener('install',e=>e.waitUntil(caches.open(CACHE).then(c=>c.addAll(CORE)).then(()=>self.skipWaiting())));
self.addEventListener('activate',e=>e.waitUntil(caches.keys().then(keys=>Promise.all(keys.filter(k=>k!==CACHE).map(k=>caches.delete(k)))).then(()=>self.clients.claim())));
self.addEventListener('fetch',e=>{const r=e.request;if(r.method!=='GET'||new URL(r.url).origin!==location.origin)return;if(r.mode==='navigate'){e.respondWith(fetch(r).then(x=>{const y=x.clone();caches.open(CACHE).then(c=>c.put(HOME,y));return x}).catch(()=>caches.match(HOME)));return}e.respondWith(caches.match(r).then(hit=>hit||fetch(r).then(x=>{if(x.ok){const y=x.clone();caches.open(CACHE).then(c=>c.put(r,y))}return x})))});
self.addEventListener('push',e=>{let d={};try{d=e.data?e.data.json():{}}catch{d={body:e.data?.text()||''}}const icon=new URL('icon.svg',BASE).href;e.waitUntil(self.registration.showNotification(d.title||'DropRadar',{body:d.body||'',icon,badge:icon,tag:d.tag||'dropradar',renotify:true,data:{url:d.url||'dropradar.html'}}))});
self.addEventListener('notificationclick',e=>{e.notification.close();let u=String(e.notification.data?.url||'dropradar.html');if(u.startsWith('/?'))u='dropradar.html'+u.slice(1);else if(u==='/'||u==='./')u='dropradar.html';const target=new URL(u,BASE).href;e.waitUntil(clients.matchAll({type:'window',includeUncontrolled:true}).then(list=>{for(const c of list){if(c.url.startsWith(BASE.href)&&'focus'in c){c.navigate(target);return c.focus()}}return clients.openWindow(target)}))});
