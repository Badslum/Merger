async function getData() {
    try {
        const resp = await fetch("/data");
        if (!resp.ok) {
            console.log("HTTP", resp.status);
            return;
        }
        const data = await resp.json();
        if (data) {
            updateUI(data);
        }
        return;
    } catch (ex) {
        console.log("Fetch error:", ex);
        updateSky(undefined);
        return;
    }
}

function updateUI(data) {
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

    updateSky(data.lux ?? 1000);
}

function updateSky(lux) {
    const day = document.getElementById("day-sky");
    const night = document.getElementById("night-sky");
    if (lux > 500 || lux === undefined) {
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

    let targetBlocks = Math.floor((rows * cols) * 0.8);
    let filled = 0;

    while (queue.length > 0 && filled < targetBlocks) {
        
        const [row, col] = queue.shift();
        if (Math.random() < 0.004) break;

        for (const [dx, dy] of dirs) {
            const x = row + dx;
            const y = col + dy;

            if (x < 0 || x >= rows) continue;
            if (y < 0 || y >= cols) continue;
            if (grid[x][y] !== 0) continue;
            if (Math.random() < 0.25) continue;

            grid[x][y] = 1;
            const rect = document.createElementNS(svgNS, "rect");

            const level = 1 + Math.floor(Math.random() * 5);
            const opacity = 0.25 + level * 0.15;

            rect.setAttribute("x", y);
            rect.setAttribute("y", x);
            rect.setAttribute("width", 1);
            rect.setAttribute("height", 1);
            rect.setAttribute("fill", `rgba(255,255,255,${opacity})`);

            cloud.appendChild(rect);
            queue.push([x,y]);

            filled++;
        }
    }
    cloud.setAttribute("transform", `translate(${posX}, ${posY})`);
    return cloud;
}

function spawnStars(count=128) {
    const svg = document.getElementById("back");
    const group = document.getElementById("stars");

    for (let i = 0; i < count; i++) {
        const x = Math.random() * 256;
        const y = Math.random() * 142;

        const e = document.createElementNS("http://www.w3.org/2000/svg", "rect");
        e.setAttribute("class", "star");
        e.setAttribute("x", x);
        e.setAttribute("y", y);
        e.setAttribute("width", 1);
        e.setAttribute("height", 1);
        e.setAttribute("fill", "yellow");
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

document.addEventListener('DOMContentLoaded', () => {
    spawnClouds(32);
    spawnStars(128);
    setInterval(getData, 1000);
});