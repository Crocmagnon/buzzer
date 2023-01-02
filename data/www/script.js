const GLOBAL_TIMEOUT = 7000;

function play() {
    console.log("Play...");
    fetch("/play", { signal: AbortSignal.timeout(GLOBAL_TIMEOUT) });
}

function volume(modifier) {
    const body = new FormData();
    body.set("modifier", modifier);
    fetch(`/change-volume`, { method: "POST", body: body, signal: AbortSignal.timeout(GLOBAL_TIMEOUT) })
        .then(response => response.json())
        .then(handleStatus);
}

function loadStatus() {
    console.log("Status...");
    fetch("/status", { signal: AbortSignal.timeout(GLOBAL_TIMEOUT) })
        .then(response => response.json())
        .then(handleStatus);
}

function selectFile(name) {
    console.log("Select file");
    const body = new FormData();
    body.set("fileName", name);
    fetch("/select-file", { method: "POST", body: body, signal: AbortSignal.timeout(GLOBAL_TIMEOUT) })
        .then(response => response.json())
        .then(handleStatus);
}

function handleStatus(data) {
    console.log("data", data);
    let dom = "";
    data.files.forEach(element => {
        if (element === data.selectedFile) {
            dom += `<button class="w3-button w3-green" onclick="selectFile('${element}')">${element}</button>`;
        }
        else {
            dom += `<button class="w3-button w3-blue" onclick="selectFile('${element}')">${element}</button>`;
        }
    });
    document.getElementById("available-files").innerHTML = dom;
    document.getElementById("volume-current").innerText = data.volume.current;
    document.getElementById("volume-increase").disabled = !data.volume.canIncrease;
    document.getElementById("volume-decrease").disabled = !data.volume.canDecrease;
}

(() => {
    loadStatus();
    setInterval(loadStatus, 10000);
})();
