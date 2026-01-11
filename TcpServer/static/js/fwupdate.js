const uploadForm = document.getElementById('uploadForm');
const fileInput = document.getElementById('fileInput');
const messageDiv = document.getElementById('message');
const updateSection = document.getElementById('updateSection');
const startUpdateBtn = document.getElementById('startUpdateBtn');
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
   Trigger ESP32 firmware update
   ========================= */
startUpdateBtn.addEventListener('click', async () => {
    if (!firmwareUploaded) {
        updateStatus.textContent = 'Upload firmware first';
        return;
    }

    updateStatus.textContent = 'Sending firmware update command to ESP32...';

    try {
        const resp = await fetch('/api?cmd=fwupdate');
        const data = await resp.json();

        if (!resp.ok || data.status !== 'ok') {
            updateStatus.textContent = 'Failed to trigger update';
            return;
        }

        updateStatus.textContent =
            'Firmware update triggered. ESP32 is downloading and updating...';

    } catch (err) {
        console.error(err);
        updateStatus.textContent = 'Error triggering update';
    }
});
