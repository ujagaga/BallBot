const uploadForm = document.getElementById('uploadForm');
const fileInput = document.getElementById('fileInput');
const messageDiv = document.getElementById('message');
const updateSection = document.getElementById('updateSection');
const startUpdateBtn = document.getElementById('startUpdateBtn');
const progressBar = document.getElementById('progressBar');
const progressText = document.getElementById('progressText');
const updateStatus = document.getElementById('updateStatus');

let firmwareUploaded = false;

/* =========================
   Upload firmware to server
   ========================= */
uploadForm.addEventListener('submit', async (e) => {
    e.preventDefault();

    const file = fileInput.files[0];
    if (!file) {
        messageDiv.textContent = 'Please select a firmware file';
        return;
    }

    firmwareUploaded = false;
    startUpdateBtn.disabled = true;
    updateSection.style.display = 'none';

    const formData = new FormData();
    formData.append('file', file);

    messageDiv.textContent = 'Uploading firmware to server...';

    try {
        const resp = await fetch('/upload_fw', {
            method: 'POST',
            body: formData
        });

        const data = await resp.json();

        if (!resp.ok) {
            messageDiv.textContent = data.error || 'Upload failed';
            return;
        }

        firmwareUploaded = true;
        messageDiv.textContent = data.message || 'Firmware uploaded successfully';

        updateSection.style.display = 'block';
        startUpdateBtn.disabled = false;

    } catch (err) {
        console.error(err);
        messageDiv.textContent = 'Upload error';
    }
});

/* =========================
   Start ESP32 update
   ========================= */
startUpdateBtn.addEventListener('click', async () => {
    if (!firmwareUploaded) {
        updateStatus.textContent = 'Upload firmware first';
        return;
    }

    updateStatus.textContent = 'Starting update...';
    progressBar.value = 0;
    progressText.textContent = '0%';

    try {
        const resp = await fetch('/start_update', { method: 'POST' });
        if (!resp.ok) {
            updateStatus.textContent = 'Failed to start update';
            return;
        }

        updateStatus.textContent = 'Updating...';

        const evtSource = new EventSource('/update_progress');

        evtSource.onmessage = (event) => {
            const msg = event.data;

            if (msg.startsWith('PROGRESS:')) {
                const percent = parseInt(msg.split(':')[1], 10);
                progressBar.value = percent;
                progressText.textContent = percent + '%';
            } else if (msg === 'DONE') {
                progressBar.value = 100;
                progressText.textContent = '100%';
                updateStatus.textContent = 'Update complete!';
                evtSource.close();
            } else if (msg === 'ERR update') {
                updateStatus.textContent = 'Update failed!';
                evtSource.close();
            }
        };

        evtSource.onerror = () => {
            updateStatus.textContent = 'Connection lost';
            evtSource.close();
        };

    } catch (err) {
        console.error(err);
        updateStatus.textContent = 'Update error';
    }
});
