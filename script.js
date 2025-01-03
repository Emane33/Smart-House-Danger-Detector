const gasData = [];
const soundData = [];
const alerts = document.getElementById('alerts');
let securityTime = null;

// WebSocket connection to the ESP32
const socket = new WebSocket('ws://192.168.100.72:81');

socket.onopen = function(event) {
    console.log('WebSocket connection established');
};

socket.onmessage = function(event) {
    console.log('Data received from ESP32:', event.data);
    try {
        if (event.data.startsWith('ALERT:')) {
            // Handle alert messages
            showModal(event.data);
        } else if (event.data === 'CLOSE_MODAL') {
            // Handle close modal message
            console.log('Received CLOSE_MODAL message');
        } else {
            // Handle sensor data
            const data = event.data.split(': ');
            if (data.length < 2) {
                throw new Error('Incomplete data received');
            }
            const key = data[0].trim();
            const value = parseInt(data[1].trim());

            if (isNaN(value)) {
                throw new Error('Invalid data format');
            }

            if (key === 'Gas') {
                updateCharts({ gas: value });
            } else if (key === 'Sound') {
                updateCharts({ sound: value });
            } else {
                throw new Error('Unknown data key');
            }
        }
    } catch (error) {
        console.error('Error parsing data:', error);
    }
};

const gasChart = new Chart(document.getElementById('gasChart'), {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Gas Level',
            data: gasData,
            borderColor: 'green',
            fill: false
        }]
    },
    options: {
        responsive: false,
        maintainAspectRatio: false,
        scales: {
            y: {
                beginAtZero: true
            }
        }
    }
});

const soundChart = new Chart(document.getElementById('soundChart'), {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Sound Level',
            data: soundData,
            borderColor: 'orange',
            fill: false
        }]
    },
    options: {
        responsive: false,
        maintainAspectRatio: false,
        scales: {
            y: {
                beginAtZero: true
            }
        }
    }
});

function setSecurityTime() {
    securityTime = document.getElementById('securityTime').value;
    console.log(`Security time set to: ${securityTime}`);
    if (socket.readyState === WebSocket.OPEN) {
        socket.send(securityTime); // Send the security time to the ESP32
    } else {
        console.error('WebSocket is not open. Ready state: ' + socket.readyState);
    }
}

function showModal(message) {
    const modal = document.getElementById('alertModal');
    const alertMessage = document.getElementById('alertMessage');
    alertMessage.textContent = message;
    modal.style.display = 'block';
}

function closeModal() {
    const modal = document.getElementById('alertModal');
    modal.style.display = 'none';
    if (socket.readyState === WebSocket.OPEN) {
        socket.send('OK'); // Send acknowledgment to the ESP32
    } else {
        console.error('WebSocket is not open. Ready state: ' + socket.readyState);
    }
}

function updateCharts(data) {
    console.log('Updating charts with data:', data);

    if (data.gas !== undefined) {
        gasData.push(data.gas);
        gasChart.data.labels.push('');
        if (data.gas > 5000) {
            showModal('Danger! High gas level');
        }
        gasChart.update();
    }

    if (data.sound !== undefined) {
        soundData.push(data.sound);
        soundChart.data.labels.push('');
        const currentTime = new Date().toTimeString().slice(0, 5);
        if (securityTime && currentTime >= securityTime) {
            if (data.sound > 50) {
                showModal('Someone got in!');
            }
            document.getElementById('soundChart').style.display = 'block';
            document.getElementById('sleepMessage').style.display = 'none';
        } else {
            document.getElementById('soundChart').style.display = 'none';
            document.getElementById('sleepMessage').style.display = 'block';
        }
        soundChart.update();
    }
}
