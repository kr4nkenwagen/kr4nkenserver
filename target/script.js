// Select the button
const button = document.querySelector('.primary-btn');

// Function to generate confetti
function createConfetti() {
    const confettiCount = 50;
    for (let i = 0; i < confettiCount; i++) {
        const confetti = document.createElement('div');
        confetti.classList.add('confetti');
        confetti.style.left = Math.random() * window.innerWidth + 'px';
        confetti.style.backgroundColor = `hsl(${Math.random() * 360}, 70%, 60%)`;
        confetti.style.animationDuration = 1 + Math.random() * 2 + 's';
        document.body.appendChild(confetti);

        // Remove confetti after animation
        confetti.addEventListener('animationend', () => {
            confetti.remove();
        });
    }
}

// Button click event
button.addEventListener('click', () => {
    createConfetti();

    // Temporarily change button color
    const originalColor = button.style.backgroundColor;
    button.style.backgroundColor = '#e94e77';
    setTimeout(() => {
        button.style.backgroundColor = '';
    }, 500);
});
