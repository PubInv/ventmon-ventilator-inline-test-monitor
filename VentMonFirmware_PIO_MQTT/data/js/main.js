async function refresh(){
  const s=await (await fetch('/api/status')).json();
  net.innerHTML=`WiFi: <b class="${s.wifi?'ok':'bad'}">${s.ip}</b> MQTT: <b class="${s.mqtt?'ok':'bad'}">${s.mqtt?'connected':'offline'}</b>`;
  meas.textContent=JSON.stringify(s.lastMeasurement,null,2);
  alarm.textContent=s.lastAlarm||'';
  lines.textContent=(s.line1||'')+'\n'+(s.line2||'');
}
refresh();
setInterval(refresh,1000);
