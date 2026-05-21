document.getElementById("nav-report").addEventListener('click',() => {
    console.log("REPORT CLICK");
    document.getElementById("weather-report").hidden = false;
    document.getElementById("weather-trend").hidden = true;
    startPolling("/data",updateMeters,2000);
});

document.getElementById("nav-trend").addEventListener('click', () => {
    document.getElementById("weather-report").hidden = true;
    document.getElementById("weather-trend").hidden = false;
    startPolling("/trend",updateTrend,15000);
});

let poller = null;
function startPolling(endpoint = "/data", fn=updateMeters, interval=2000) {
    stopPolling();
    poller = setInterval(async () => {
        try {
            const resp = await fetch(endpoint);
            if (!resp.ok) {
                console.log("HTTP", resp.status);
                return;
            }
            const data = await resp.json();
            fn(data);
        } catch (ex) {
            console.log("Fetch error:", ex);
        }
    }, interval);
}

function stopPolling() {
    if (poller !== null) {
        clearInterval(poller);
        poller = null;
    }
}

function updateTrend(data) {
    
    const values = data.values;

    if (!values || values.length === 0) return;

    const maxTemp = 40;
    const minTemp = -10;
    const maxHum = 100;
    const minHum = 0;
    const width = 128;
    const height = 32;

    const tempPoints = [];
    const humPoints = [];
    const tooltips = document.getElementById("trend-tooltips");
    tooltips.innerHTML = ""; 

    for (let i = 0; i < values.length; i++){
        const x = (i / (values.length - 1)) * width;

        const temp = values[i][1];
        const yT = height - ((temp - tempMin) / (tempMax - tempMin)) * height;
        tempPoints.push(`${x},${yT}`);
        
        const tText = document.createElementNS("http://www.w3.org/2000/svg", "text");
        tText.setAttribute("x", x + 2);
        tText.setAttribute("y", yT - 2);
        tText.setAttribute("font-size", "2");
        tText.setAttribute("fill", "white");
        tText.setAttribute("visibility", "hidden");
        tText.textContent = `${temp}°C`;
        
        const tpt = makeRect("tp", x, 0, "transparent");
        tpt.addEventListener("mouseenter", () => {
            tpt.setAttribute("fill", "blue");
            tText.setAttribute("visibility", "visible");
        });
        tpt.addEventListener("mouseleave", () => {
            tpt.setAttribute("fill", "transparent");
            tText.setAttribute("visibility", "hidden");
        });

        const hum = values[i][2];
        const yH = height - ((hum - humMin) / (humMax - humMin)) * height;
        humPoints.push(`${x},${yH}`);

        const hText = document.createElementNS("http://www.w3.org/2000/svg", "text");
        hText.setAttribute("x", x + 2);
        hText.setAttribute("y", yT - 2);
        hText.setAttribute("font-size", "2");
        hText.setAttribute("fill", "white");
        hText.setAttribute("visibility", "hidden");
        hText.textContent = `${temp}°C`;
        
        const hpt = makeRect("hp", x, yH, "transparent");
        tpt.addEventListener("mouseenter", () => {
            tpt.setAttribute("fill", "blue");
            hText.setAttribute("visibility", "visible");
        });
        tpt.addEventListener("mouseleave", () => {
            tpt.setAttribute("fill", "transparent");
            hText.setAttribute("visibility", "hidden");
        });
    }

    document.getElementById("trend-temp").setAttribute("points", tempPoints.join(" "));
    document.getElementById("trend-hum").setAttribute("points", humPoints.join(" "));
}

function updateMeters(data) {
    document.getElementById("temp-meter").value = data.temp;
    document.getElementById("temp-value").textContent = data.temp.toFixed(1);

    document.getElementById("dp-meter").value = data.dp;
    document.getElementById("dp-value").textContent = data.dp.toFixed(1);

    document.getElementById("hum-meter").value = data.hum;
    document.getElementById("hum-value").textContent = data.hum.toFixed(0);

    document.getElementById("absHum-meter").value = data.absHum;
    document.getElementById("absHum-value").textContent = data.absHum.toFixed(1);

    document.getElementById("pres-meter").value = data.pres;
    document.getElementById("pres-value").textContent = data.pres.toFixed(0);

    document.getElementById("wb-meter").value = data.wb;
    document.getElementById("wb-value").textContent = data.wb.toFixed(1);

    updateSky(data.lux ?? undefined);
}

function updateSky(lux) {
    const day = document.getElementById("day-sky");
    const night = document.getElementById("night-sky");
    if (lux > 480 || lux === undefined) {
        document.body.classList.remove("night");
        day.style.display = "block";
        night.style.display = "none";
        document.body.classList.add("day");
    } else {
        document.body.classList.remove("day");
        day.style.display = "none";
        night.style.display = "block";
        document.body.classList.add("night");
    }
}

function spawnSun() {
    const svgNS = "http://www.w3.org/2000/svg";
    const sun = document.getElementById("sun");
    const cx = 220;
    const cy = 25;
    const r = 14;

    for (let y = -r; y <= r; y++) {
        for (let x = -r; x <= r; x++) {
            if (x*x + y*y <= r*r) {
                const e = makeRect("sun",cx+x,cy+y)
                sun.appendChild(e);
            }
        }
    }
}

function spawnClouds(count = 16) {
    const cloudsA = document.getElementById("clouds-a");
    const cloudsB = document.getElementById("clouds-b");

    let x = -32;
    let y = 16;

    for (let i = 0; i < count; i++) {
        x += 12 + Math.random() * 32;
        y = 16 + Math.random() * 8;

        if (x > 255) continue;

        const cloud = createCloud(x, y);
        cloudsA.appendChild(cloud);
        const clone = cloud.cloneNode(true);
        cloudsB.appendChild(clone);
    }
}

function createCloud(posX, posY) {
    const svgNS = "http://www.w3.org/2000/svg";
    const cloud = document.createElementNS(svgNS, "g");

    const rows = 16 + Math.floor(Math.random() * 4);
    const cols = 32 + Math.floor(Math.random() * 8);
    const grid = Array.from({ length: rows }, () =>
        Array(cols).fill(0)
    );
    const dirs = [[0, 1],[1, 0],[0, -1],[-1, 0]];

    const startRow = Math.floor(rows/2);
    const startCol = Math.floor(cols/2);
    const queue = [[startRow, startCol]];

    let targetBlocks = Math.floor((rows * cols) * 0.6);
    let filled = 0;

    while (queue.length > 0 && filled < targetBlocks) {
        
        const [row, col] = queue.shift();
        if (Math.random() < 0.006) break;

        for (const [dx, dy] of dirs) {
            const x = row + dx;
            const y = col + dy;

            if (x < 0 || x >= rows) continue;
            if (y < 0 || y >= cols) continue;
            if (grid[x][y] !== 0) continue;
            if (Math.random() < 0.25) continue;

            grid[x][y] = 1;
            const level = 1 + Math.floor(Math.random() * 5);
            const opacity = 0.25 + level * 0.15;
            const e = makeRect("cloud",y,x,`rgba(255,255,255,${opacity})`);
            
            cloud.appendChild(e);
            queue.push([x,y]);
            filled++;
        }
    }
    cloud.setAttribute("transform", `translate(${posX}, ${posY})`);
    return cloud;
}

function spawnMoon() {
    const svgNS = "http://www.w3.org/2000/svg";
    const moon = document.getElementById("moon");

    const cx = 220, cy = 25;
    const rOuter = 10;
    const rInner = 9;
    const offsetX = -5;

    for (let y = -rOuter; y <= rOuter; y++) {
        for (let x = -rOuter; x <= rOuter; x++) {
            const outer = (x*x + y*y <= rOuter*rOuter);
            const sx = x - offsetX;
            const inner = (sx*sx + y*y <= rInner*rInner);

            if (outer) {
                let color = "#04044e";
                if (!inner) {
                    color = "#cad4f5";

                    const dist = Math.sqrt(x*x + y*y) / rOuter;
                    if (Math.random() < 0.16 && dist < 0.8) {
                        color = "#a6adc9";
                    }
                }
                const e = makeRect("moon",cx+x,cy+y,color);
                moon.appendChild(e);
            }
        }
    }
}

function spawnStars(count=128) {
    const svg = document.getElementById("back");
    const group = document.getElementById("stars");

    for (let i = 0; i < count; i++) {
        const x = Math.random() * 256;
        const y = Math.random() * 142;
        const e = makeRect("star",x,y);
        group.appendChild(e);
    }
    initTwinkle()
}

function initTwinkle() {
    const stars = document.querySelectorAll(".star");

    const data = [...stars].map(el => ({
        el,
        speed: 0.5 + Math.random(),
        phase: Math.random() * Math.PI * 2
    }));

    function animate(t) {
        for (const s of data) {
            const o = 0.3 + Math.sin(t * 0.001 * s.speed + s.phase) * 0.7;
            s.el.style.opacity = o;
        }
        requestAnimationFrame(animate);
    }

    requestAnimationFrame(animate);
}

function makeRect(group = "", x = 0, y = 0,color="#f3eb1f", width=1, height=1){
    const e = document.createElementNS("http://www.w3.org/2000/svg", "rect");
    e.setAttribute("class", group);
    e.setAttribute("x", x);
    e.setAttribute("y", y);
    e.setAttribute("width", width);
    e.setAttribute("height", height);
    e.setAttribute("fill", color);
    return e;
}

document.addEventListener('DOMContentLoaded', () => {
    document.getElementById("nav-report").click();

    spawnSun();
    spawnClouds(32);
    
    spawnMoon();
    spawnStars(128);
});