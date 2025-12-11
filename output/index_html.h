const char MAIN_page[] PROGMEM = R"rawliteral(<html>
<head>
    <title>Adventskranz</title>
    <meta charset="utf-8">
    <meta name="author" content="Bastian Roth">
    <meta name="date" content="30.11.2025">
    <style>
/* General Style */
*{
  box-sizing: border-box;
  margin: 0;
  padding: 0;
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
}

*:focus {
    outline: none;
}

body {
  background-color: #121212;
  color: #f0f0f0;
  text-align: center;
  user-select: none;
}

h1 {
    color: gold;
}

/* Button Styles */
button {
  background-color: #1e1e1e;
  color: #f0f0f0;
  border: 2px solid #f0f0f0;
  border-radius: 4px;
  padding: 10px 20px;
  cursor: pointer;
}

button.action:hover {
  background: #1e1e1e;
}

button.lit {
  background-color: gold;
  color: #121212;
  border: 2px solid gold;
  border-radius: 4px;
  padding: 10px 20px;
}

button.lit:hover {
  background-color: #ffd700;
}

/* Div Styles */
.header {
  display: flex;
  position: fixed;
  top: 0%;
  left: 0%;
  height: 20%;
  width: 100;
  align-items: center;
  align-content: center;
  justify-content: center;
  text-align: center;
}
.advent-wreath{
  display: flex;
  position: fixed;
  top: 20%;
  left: 0%;
  height: 80%;
  width: 100%;
  align-items: center;
  align-content: center;
  justify-content: center;
  text-align: center;
}

.date-box {
  display: flex;
  position: fixed;
  top: 20%;
  right: 0%;
  height: 20%;
  width: auto;
  align-items: center;
  justify-content: center;
  text-align: center;
}
</style>
    <script>
// Getting the current date and updating the header
function getDate() {
    const dateElement = document.getElementById('date');
    const today = new Date();
    const options = { year: 'numeric', month: 'long', day: 'numeric' };
    dateElement.textContent = today.toLocaleDateString('de-DE', options);
    if (today.getDate() >= 24 && today.getMonth() === 11) {
        document.querySelector('h1').textContent = "Fröhliche Weihnachten!";
    }
    return today;
}
// Calculate the first Advent Sunday for a given year
function getFirstAdvent(year) {
  let christmas = new Date(year, 11, 25);
  let firstAdvent = new Date(christmas);
  firstAdvent.setDate(christmas.getDate() - 28);
  while (firstAdvent.getDay() !== 0) {
    firstAdvent.setDate(firstAdvent.getDate() + 1);
  }
  return firstAdvent;
}
// Light candles based on the currentdate or user interaction
function lightcandles() {
    const leds = ['led1','led2','led3','led4'];
    const today = getDate();
    const start = getFirstAdvent(today.getFullYear());
    const diff = Math.floor((today - start) / (1000 * 60 * 60 * 24));
    const candles = Math.min(4, Math.floor(diff/7) +1);
    // Updating header to include the current Advent week
    if (candles > 0 && today.getDate() <= 24) {
        document.querySelector('h1').textContent = "Fröhlichen " + candles + ". Advent!";
    } 
    // Auto toggle candles
    for (let i = 0; i < candles; i++) {
        document.getElementById(`led${i+1}`).classList.add('lit');
        fetch(`/toggle?led=led${i+1}&state=true`);
    }
    // Manual toggle candles
    leds.forEach(function(i) {
        const led = document.getElementById(i);
        led.addEventListener('click', () => {
            led.classList.toggle('lit');
            i--;
            fetch(`/toggle?led=${i+1}&state=${led.classList.contains('lit')}`);
        });
    });
    // sync led states from board
    fetch('/state').then(r => r.json()).then(states => {
    leds.forEach((id, i) => {
        const led = document.getElementById(id);
        if (states[i]) led.classList.add('lit'); else led.classList.remove('lit');
    });
})
}
// Initialize when DOM is fully loaded
document.addEventListener('DOMContentLoaded', () => {
    lightcandles();
});
</script>
</head>
<body>
    <div id="header">
        <h1> Elektronischer Adventskranz </h1>
    </div>
    <div id="advent-wreath">
        <br>
        <button id="led1">Kerze 1</button>
        <button id="led2">Kerze 2</button>
        <button id="led3">Kerze 3</button>
        <button id="led4">Kerze 4</button>
        <br>
    </div>
    <div id="date-box">
        <p id="date-text">Heutiges Datum:</p>
        <p id="date"></p>
    </div>
</body>
</html>)rawliteral";