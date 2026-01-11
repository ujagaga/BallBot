const uploadForm = document.getElementById('uploadForm');
const fileInput = document.getElementById('fileInput');
const messageDiv = document.getElementById('message');
const updateSection = document.getElementById('updateSection');
const startUpdateBtn = document.getElementById('startUpdateBtn');
const progressBar = document.getElementById('progressBar');
const progressText = document.getElementById('progressText');
const updateStatus = document.getElementById('updateStatus');

// Handle file upload
uploadForm.addEventListener('submit', async (e) => {
e.preventDefault();
const file = fileInput.files[0];
if (!file) return;

const formData = new FormData();
formData.append('file', file);

messageDiv.textContent = 'Uploading firmware to server...';

const resp = await fetch('/upload_fw', {
  method: 'POST',
  body: formData
});

const text = await resp.text();
messageDiv.innerHTML = text;

if (resp.ok) {
  // Show update section after successful upload
  updateSection.style.display = 'block';
}
});

// Handle firmware update via TCP
startUpdateBtn.addEventListener('click', async () => {
    updateStatus.textContent = 'Updating...';
    progressBar.value = 0;
    progressText.textContent = '0%';

    // Start firmware update on ESP32
    const resp = await fetch('/start_update', { method: 'POST' });
    if (!resp.ok) {
        updateStatus.textContent = 'Failed to start update';
        return;
    }

    // Listen for SSE progress
    const evtSource = new EventSource('/update_progress');

    evtSource.onmessage = (event) => {
        const msg = event.data;

        if (msg.startsWith("PROGRESS:")) {
            const percent = parseInt(msg.split(":")[1]);
            progressBar.value = percent;
            progressText.textContent = percent + '%';
        } else if (msg === "DONE") {
            progressBar.value = 100;
            progressText.textContent = '100%';
            updateStatus.textContent = 'Update complete!';
            evtSource.close();
        } else if (msg === "ERR update") {
            updateStatus.textContent = 'Update failed!';
            evtSource.close();
        }
    };

    evtSource.onerror = (err) => {
        console.error("SSE error", err);
        evtSource.close();
    };
});
