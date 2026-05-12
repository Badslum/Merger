function navInit(btn, fn) {
    const b = document.getElementById(btn);
    b.addEventListener('click', fn);
}

function loadReport(){}
function loadTrend(){}
function toggleTheme(){}

async function poll() {
    const data = await getData();
    if (data) updateUI(data);
}

async function getData() {
    try {
        const resp = await fetch("/data");
        if (!resp.ok) {
            console.log("HTTP", resp.status);
            return null;
        }
        return await resp.json();
    } catch (e) {
        console.log("Fetch error:", e);
        return null;
    }
}

const METRICS = {
    temp:  { id: "temp",  min: -20, max: 50,  label: "Temperatur" },
    dp:    { id: "dp",    min: -20, max: 50,  label: "Taupunkt" },
    hum:   { id: "hum",   min: 0,   max: 100, label: "Luftfeuchte" },
    absHum:{ id: "hum",   min: 0,   max: 50,  label: "Absolute Feuchte" },
    pres:  { id: "pres",  min: 900, max: 1100,label: "Luftdruck" },
    wb:    { id: "wb",    min: -20, max: 50,  label: "Feuchtkugel" }
};

function uiInit() {
    for (const key in METRICS) {
        const m = METRICS[key];
        const container = document.getElementById(m.id);

        const label = document.createElement("span");
        label.className = "label";
        label.textContent = m.label;

        const meter = document.createElement("meter");
        meter.id = `${key}-meter`;
        meter.min = m.min;
        meter.max = m.max;

        const value = document.createElement("span");
        value.id = `${key}-value`;
        value.className = "value";

        container.appendChild(label);
        container.appendChild(meter);
        container.appendChild(value);
    }
}

function updateUI(data) {
    for (const key in METRICS) {
        const meter = document.getElementById(`${key}-meter`);
        const value = document.getElementById(`${key}-value`);

        if (!meter || !value) continue;

        const v = data[key];
        meter.value = v;
        value.textContent = v.toFixed(1);
    }
}

document.addEventListener('DOMContentLoaded', () => {
    navInit('nav-report', loadReport);
    navInit('nav-trend', loadTrend);
    navInit('opt-theme', toggleTheme);
    uiInit();
    setInterval(poll, 1000);
});