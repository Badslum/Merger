async function poll() {
    const data = await getData();
    if (data) {
        updateUI(data);
    }
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

    updateSky(data.lux);
}

function updateSky(lux) {
    const day = document.getElementById("day-layer");
    const night = document.getElementById("night-layer");
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

function spawnClouds(count = 8) {
    const cloudsLayer = document.getElementById("clouds");

    let x = 300;
    let y = 10;

    for (let i = 0; i < count; i++) {
        x += Math.random() * 200;
        y += Math.random() * 20;

        const cloud = createCloud(x, y);
        cloudsLayer.appendChild(cloud);
        x += 10;
    }
}

function createCloud(posX, posY) {
    const svgNS = "http://www.w3.org/2000/svg";
    const cloud = document.createElementNS(svgNS, "g");
    cloud.setAttribute("class", "cloud");

    const rows = 3 + Math.floor(Math.random() * 2);
    const cols = 5 + Math.floor(Math.random() * 2);
    const blockSize = 6;

    for (let y = 0; y < rows; y++) {
        for (let x = 0; x < cols; x++) {
            if (Math.random() < 0.5) {
                const rect = document.createElementNS(svgNS, "rect");
                rect.setAttribute("x", x * blockSize);
                rect.setAttribute("y", y * blockSize);
                rect.setAttribute("width", blockSize);
                rect.setAttribute("height", blockSize);
                rect.setAttribute("fill", "#ffffffaa"); // semi-transparent

                cloud.appendChild(rect);
            }
        }
    }

    cloud.setAttribute("transform", `translate(${posX}, ${posY})`);
    return cloud;
}

function spawnStars(count=8) {
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
    spawnClouds(10);
    spawnStars(100);
    setInterval(poll, 1000);
});