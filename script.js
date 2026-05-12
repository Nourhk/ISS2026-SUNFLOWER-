document.addEventListener("DOMContentLoaded", () => {


// ── Panel location (change to your real GPS) ──
const PANEL_LAT = 36.82;
const PANEL_LON = 10.17;
const PANEL_TZ  = 1;

let currentTilt    = 45;
let currentAzimuth = 180;
let trackerEnergyAccum = 0;

function toRad(d) { return d * Math.PI / 180; }
function toDeg(r) { return r * 180 / Math.PI; }

function calcSolarPosition(date) {
    const JD = date.getTime() / 86400000 + 2440587.5;
    const JC = (JD - 2451545) / 36525;
    const L0 = (280.46646 + JC * (36000.76983 + JC * 0.0003032)) % 360;
    const M  = toRad(357.52911 + JC * (35999.05029 - 0.0001537 * JC));
    const C  = Math.sin(M) * (1.914602 - JC * (0.004817 + 0.000014 * JC))
             + Math.sin(2*M) * (0.019993 - 0.000101 * JC)
             + Math.sin(3*M) * 0.000289;
    const sunLon = toRad(L0 + C);
    const e = toRad(23.439291111 - JC * (0.013004167 + JC * 0.0000001638));
    const declination = Math.asin(Math.sin(e) * Math.sin(sunLon));
    const y = Math.tan(e/2)**2;
    const eqTime = 4 * toDeg(y*Math.sin(toRad(2*L0)) - 2*0.016708634*Math.sin(2*M)
        + 4*0.016708634*y*Math.sin(2*M)*Math.cos(toRad(2*L0))
        - 0.5*y*y*Math.cos(toRad(4*L0)) - 1.25*(0.016708634**2)*Math.sin(4*M));
    const hours = date.getHours() + date.getMinutes()/60 + date.getSeconds()/3600;
    const hourAngle = toRad(((hours*60 + eqTime + 4*PANEL_LON - 60*PANEL_TZ)/4) - 180);
    const latRad = toRad(PANEL_LAT);
    const cosZ = Math.sin(latRad)*Math.sin(declination) + Math.cos(latRad)*Math.cos(declination)*Math.cos(hourAngle);
    const zenith = Math.acos(Math.max(-1, Math.min(1, cosZ)));
    const elevation = 90 - toDeg(zenith);
    const sinAz = -(Math.cos(declination)*Math.sin(hourAngle)) / Math.cos(zenith);
    const cosAz = (Math.sin(declination) - Math.sin(latRad)*cosZ) / (Math.cos(latRad)*Math.sin(zenith));
    const azimuth = toDeg(Math.atan2(sinAz, cosAz)) + 180;
    return { elevation: parseFloat(elevation.toFixed(1)), azimuth: parseFloat(azimuth.toFixed(1)), isDaytime: elevation > 0 };
}

function getSeason() {
    const m = new Date().getMonth() + 1;
    if (m>=3&&m<=5) return "🌸 Spring";
    if (m>=6&&m<=8) return "☀️ Summer";
    if (m>=9&&m<=11) return "🍂 Autumn";
    return "❄️ Winter";
}

function updateSolarPosition() {
    const now = new Date();
    const pos = calcSolarPosition(now);
    const dateStr = now.toLocaleDateString('en-GB', {weekday:'short',day:'2-digit',month:'short',year:'numeric'});
    const timeStr = now.toLocaleTimeString('en-GB', {hour:'2-digit',minute:'2-digit',second:'2-digit'});
    const el = (id,v) => { const e=document.getElementById(id); if(e) e.textContent=v; };
    el('solar-datetime', `${dateStr}  ${timeStr}`);
    if (pos.isDaytime) {
        el('optimal-elevation', `${pos.elevation}°`);
        el('optimal-azimuth',   `${pos.azimuth}°`);
        const acc = Math.max(0, 100 - (Math.abs(currentTilt-pos.elevation) + Math.abs(currentAzimuth-pos.azimuth))/2).toFixed(1);
        el('tracking-accuracy', `${acc}%`);
        el('energy-gain', `+${(28.5*(acc/100)).toFixed(1)}%`);
    } else {
        el('optimal-elevation', 'Below horizon');
        el('optimal-azimuth',   '—');
        el('tracking-accuracy', 'Night');
        el('energy-gain',       '—');
    }
    el('solar-season-label', getSeason());
}

function updateEfficiencyPanel(powerW) {
    trackerEnergyAccum += (powerW * 2) / 3600 / 1000;
    const t = trackerEnergyAccum;
    const f = t / 1.285;
    const g = t - f;
    const p = f > 0 ? ((g/f)*100).toFixed(1) : 0;
    const el = (id,v) => { const e=document.getElementById(id); if(e) e.textContent=v; };
    el('eff-tracker', t.toFixed(3));
    el('eff-fixed',   f.toFixed(3));
    el('eff-gained',  g.toFixed(3));
    el('eff-percent', p);
    el('overall-efficiency-badge', `+${p}% more efficient`);
    const bt = document.getElementById('eff-bar-tracker');
    const bf = document.getElementById('eff-bar-fixed');
    if (bt) bt.style.width = Math.min(100,(t/5)*100) + '%';
    if (bf) bf.style.width = Math.min(100,(f/5)*100) + '%';
}// Navigation Logic
document.querySelectorAll('.nav-item').forEach(btn => {
    btn.addEventListener('click', () => {
        // Toggle Active pill
        document.querySelectorAll('.nav-item').forEach(n => n.classList.remove('active'));
        btn.classList.add('active');
        
        // Hide all views
        document.querySelectorAll('.view').forEach(v => v.classList.remove('active'));
        
        // Show target View
        const target = btn.getAttribute('data-target');
        const viewEl = document.getElementById('view-' + target);
        if (viewEl) viewEl.classList.add('active');
    });
});

// Chart globals
let historyChart, tiltChart, weeklyChart, efficiencyChart;

function initCharts() {
    initHistoryChart();
    initTiltChart();
    initWeeklyChart();
    initEfficiencyChart();
}

function initHistoryChart() {
    const ctx = document.getElementById('historyChart').getContext('2d');
    
    // Gradients
    const gradPower = ctx.createLinearGradient(0, 0, 0, 400);
    gradPower.addColorStop(0, 'rgba(245, 158, 11, 0.4)');
    gradPower.addColorStop(1, 'rgba(245, 158, 11, 0)');
    
    const gradIrr = ctx.createLinearGradient(0, 0, 0, 400);
    gradIrr.addColorStop(0, 'rgba(252, 211, 77, 0.4)');
    gradIrr.addColorStop(1, 'rgba(252, 211, 77, 0)');

    // Generate matching data shape from screenshot: starts high, goes down, flat, up again
    const labels = Array.from({length: 15}, () => '11:08');
    const powerData = [700, 500, 200, 0, 0, 0, 0, 0, 0, 0, 0, 0, 300, 600, 850];
    const irrData = [150, 100, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100, 150, 180];

    historyChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: [
                {
                    label: 'Power (W)',
                    data: powerData,
                    borderColor: '#f59e0b',
                    backgroundColor: gradPower,
                    fill: true,
                    tension: 0.1,
                    pointRadius: 0,
                    pointHoverRadius: 5
                },
                {
                    label: 'Irradiance (W/m²)',
                    data: irrData,
                    borderColor: '#fcd34d',
                    backgroundColor: gradIrr,
                    fill: true,
                    tension: 0.1,
                    pointRadius: 0,
                    pointHoverRadius: 5
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: { display: false }
            },
            scales: {
                x: {
                    grid: { display: false, drawBorder: false },
                    ticks: { color: '#9ca3af', font: { family: 'Inter', size: 10 } }
                },
                y: {
                    grid: { color: '#f3f4f6', drawBorder: false, borderDash: [5, 5] },
                    ticks: { 
                        color: '#9ca3af', 
                        font: { family: 'Inter', size: 11 },
                        stepSize: 250,
                        callback: function(value) { return value + 'W'; }
                    },
                    min: 0,
                    max: 1000
                }
            }
        }
    });
}

function initTiltChart() {
    const ctx = document.getElementById('tiltChart').getContext('2d');
    
    // Half doughnut for gauge
    tiltChart = new Chart(ctx, {
        type: 'doughnut',
        data: {
            labels: ['Tilt', 'Remainder'],
            datasets: [{
                data: [45, 135], // 45 degree out of 180
                backgroundColor: ['#ffedd5', '#f3f4f6'],
                borderWidth: 0,
                circumference: 180,
                rotation: 270
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: { display: false },
                tooltip: { enabled: false }
            },
            cutout: '85%'
        },
        plugins: [{
            id: 'needle',
            afterDraw: function(chart) {
                const ctx = chart.ctx;
                const width = chart.width;
                const height = chart.height;
                const angle = Math.PI + (45 / 180) * Math.PI; // pointing to 45 deg
                
                ctx.save();
                ctx.translate(width / 2, height); // bottom center
                ctx.rotate(angle);
                
                // Draw Needle
                ctx.beginPath();
                ctx.moveTo(0, 5);
                ctx.lineTo(height * 0.7, 0);
                ctx.lineTo(0, -5);
                ctx.fillStyle = '#4b5563';
                ctx.fill();
                
                // Needle Center
                ctx.beginPath();
                ctx.arc(0, 0, 4, 0, Math.PI * 2);
                ctx.fillStyle = '#4b5563';
                ctx.fill();
                
                ctx.restore();
            }
        }]
    });
}

function initWeeklyChart() {
    const ctx = document.getElementById('weeklyChart').getContext('2d');
    
    weeklyChart = new Chart(ctx, {
        type: 'bar',
        data: {
            labels: ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'],
            datasets: [
                {
                    label: 'Tracking System',
                    data: [12, 14, 8, 15.8, 13, 16.5, 14.8],
                    backgroundColor: '#f59e0b',
                    borderRadius: 4,
                    barPercentage: 0.5
                },
                {
                    label: 'Fixed Install',
                    data: [10, 11, 7, 10.4, 9, 12.5, 11.2],
                    backgroundColor: '#f3f4f6',
                    borderRadius: 4,
                    barPercentage: 0.5
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: {
                    position: 'bottom',
                    labels: {
                        usePointStyle: true,
                        boxWidth: 8,
                        boxHeight: 8,
                        color: '#9ca3af',
                        font: { family: 'Inter' }
                    }
                }
            },
            scales: {
                x: {
                    grid: { display: false, drawBorder: false },
                    ticks: { color: '#9ca3af', font: { family: 'Inter', size: 11 } }
                },
                y: {
                    grid: { color: '#f3f4f6', drawBorder: false, borderDash: [5, 5] },
                    ticks: { 
                        color: '#9ca3af', 
                        font: { family: 'Inter', size: 11 },
                        stepSize: 5,
                        callback: function(value) { return value + 'kWh'; }
                    },
                    min: 0,
                    max: 20
                }
            }
        }
    });
}

function initEfficiencyChart() {
    const ctx = document.getElementById('efficiencyChart');
    if (!ctx) return;
    
    efficiencyChart = new Chart(ctx.getContext('2d'), {
        type: 'line',
        data: {
            labels: ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'],
            datasets: [{
                label: 'Efficiency',
                data: [12, 14, 8, 15.8, 13, 16.5, 14.8],
                borderColor: '#10b981',
                backgroundColor: '#ffffff',
                borderWidth: 2,
                pointBackgroundColor: '#ffffff',
                pointBorderColor: '#10b981',
                pointBorderWidth: 2,
                pointRadius: 4,
                tension: 0.4
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: { display: false }
            },
            scales: {
                x: {
                    grid: { display: false, drawBorder: false },
                    ticks: { color: '#9ca3af', font: { family: 'Inter', size: 11 } }
                },
                y: {
                    grid: { color: '#f3f4f6', drawBorder: false, borderDash: [5, 5] },
                    ticks: { 
                        color: '#9ca3af', 
                        font: { family: 'Inter', size: 11 },
                        stepSize: 5,
                        callback: function(value) { return value + '%'; }
                    },
                    min: 0,
                    max: 20
                }
            }
        }
    });
}

window.addEventListener('load', () => {
    initCharts();
});

// Controls Range Sliders and Toast
const orientationRange = document.querySelector('.orange-range');
const tiltRange = document.querySelector('.gray-range');
const orientationBadge = orientationRange ? orientationRange.parentElement.querySelector('.range-badge') : null;
const tiltBadge = tiltRange ? tiltRange.parentElement.querySelector('.range-badge') : null;

if (orientationRange) {
    orientationRange.addEventListener('input', (e) => {
        const val = e.target.value;
        const max = e.target.max;
        const percentage = (val / max) * 100;
        e.target.style.setProperty('--val', `${percentage}%`);
        if (orientationBadge) orientationBadge.innerText = `${val}°`;
    });
}
if (tiltRange) {
    tiltRange.addEventListener('input', (e) => {
        const val = e.target.value;
        if (tiltBadge) tiltBadge.innerText = `${val}°`;
    });
}

const stowBtn = document.getElementById('stow-btn');
const toast = document.getElementById('stow-toast');
const toastClose = document.getElementById('toast-close-btn');

if (stowBtn && toast) {
    stowBtn.addEventListener('click', () => {
        toast.classList.add('show');
        setTimeout(() => toast.classList.remove('show'), 4000);
    });
}
if (toastClose) {
    toastClose.addEventListener('click', () => toast.classList.remove('show'));
}


// update every 2 seconds
setInterval(fetchData, 2000);
// ── Logs ──────────────────────────────────────
const MOCK_LOGS = [
    { level:'info',    message:'System initialized.', timestamp: new Date(Date.now()-3600000).toISOString() },
    { level:'info',    message:'Auto tracking activated.', timestamp: new Date(Date.now()-1800000).toISOString() },
    { level:'warning', message:'Panel temperature approaching 50°C.', timestamp: new Date(Date.now()-900000).toISOString() },
    { level:'info',    message:'ESP32 connected.', timestamp: new Date(Date.now()-300000).toISOString() },
];
let allLogs = [...MOCK_LOGS];
let currentLogFilter = 'all';

function renderLogs() {
    const container = document.getElementById('log-entries');
    if (!container) return;
    const filtered = currentLogFilter==='all' ? allLogs : allLogs.filter(l=>l.level===currentLogFilter);
    if (!filtered.length) { container.innerHTML='<div class="log-empty">No entries found.</div>'; return; }
    const iconMap = { info:'fa-circle-info', warning:'fa-triangle-exclamation', error:'fa-circle-xmark' };
    const colorMap = { info:'log-info', warning:'log-warning', error:'log-error' };
    container.innerHTML = [...filtered].reverse().map(log => `
        <div class="log-entry ${colorMap[log.level]||'log-info'}">
            <i class="fa-solid ${iconMap[log.level]||'fa-circle-info'} log-icon"></i>
            <div class="log-body">
                <span class="log-message">${log.message}</span>
                <span class="log-time">${new Date(log.timestamp).toLocaleString()}</span>
            </div>
            <span class="log-badge">${log.level.toUpperCase()}</span>
        </div>`).join('');
}

document.querySelectorAll('.log-filter').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('.log-filter').forEach(b=>b.classList.remove('active'));
        btn.classList.add('active');
        currentLogFilter = btn.getAttribute('data-level');
        renderLogs();
    });
});

document.getElementById('refresh-logs-btn')?.addEventListener('click', () => {
    fetch('/logs').then(r=>r.json()).then(d=>{ allLogs=d.length?d:MOCK_LOGS; renderLogs(); }).catch(()=>renderLogs());
});





const applyBtn = document.getElementById("applyBtn");
const slider = document.querySelector(".orange-range");

console.log("ApplyBtn:", applyBtn);
console.log("Slider:", slider);

applyBtn?.addEventListener("click", () => {
    console.log("🔥 BUTTON CLICKED");

    const angle = Number(slider?.value);
    console.log("Angle:", angle);

    firebase.database().ref("servo/orientation").set(angle)
        .then(() => console.log("✅ SENT TO FIREBASE:", angle))
        .catch(err => console.error("❌ FIREBASE ERROR:", err));
});


});