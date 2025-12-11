// Getting the current date and updating the header
function getDate() {
    const dateElement = document.getElementById('date');
    const today = new Date();
    const options = { year: 'numeric', month: 'long', day: 'numeric' };
    dateElement.textContent = today.toLocaleDateString('de-DE', options);
    if (today.getDate() >= 24 && today.getMonth() === 11) {
        document.querySelector('h1').textContent = "Fr&ouml;hliche Weihnachten!";
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
        document.querySelector('h1').textContent = "Fr&ouml;hlichen " + candles + ". Advent!";
    } 
    // Auto toggle candles
    for (let i = 0; i < candles; i++) {
    document.getElementById(`led${i+1}`).classList.add('lit');
    fetch(`/toggle?led=led${i+1}&state=true`);
    }
    // Manual toggle candles
    leds.forEach(function(id) {
        const led = document.getElementById(id);
        led.addEventListener('click', () => {
            led.classList.toggle('lit');
            fetch(`/toggle?led=${id}&state=${led.classList.contains('lit')}`);
        });
    });
    // sync led states from board
    fetch('/state').then(r => r.json()).then(states => {
    leds.forEach((id, i) => {
      const led = document.getElementById(id);
      if (states[i]) led.classList.add('lit'); else led.classList.remove('lit');
    });
});
}
// Initialize when DOM is fully loaded
document.addEventListener('DOMContentLoaded', () => {
    lightcandles();
});