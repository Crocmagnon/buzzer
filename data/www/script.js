const GLOBAL_TIMEOUT = 10000;
let connectionOk = true;

function play() {
    console.log("Play...");
    fetch("/play", { signal: AbortSignal.timeout(GLOBAL_TIMEOUT) })
        .catch(handleError);
}

function stop() {
    console.log("Stop...");
    fetch("/stop", { signal: AbortSignal.timeout(GLOBAL_TIMEOUT) })
        .catch(handleError);
}

function volume(modifier) {
    const body = new FormData();
    body.set("modifier", modifier);
    fetch(`/change-volume`, { method: "POST", body: body, signal: AbortSignal.timeout(GLOBAL_TIMEOUT) })
        .then(response => response.json())
        .then(handleStatus)
        .catch(handleError);
}

function loadStatus() {
    console.log("Status...");
    fetch("/status", { signal: AbortSignal.timeout(GLOBAL_TIMEOUT) })
        .then(response => response.json())
        .then(handleStatus)
        .catch(handleError);
}

function selectFile(name) {
    console.log("Select file");
    const body = new FormData();
    body.set("fileName", name);
    fetch("/select-file", { method: "POST", body: body, signal: AbortSignal.timeout(GLOBAL_TIMEOUT) })
        .then(response => response.json())
        .then(handleStatus)
        .catch(handleError);
}

function handleStatus(data) {
    document.body.classList.remove("w3-disabled");
    if (!connectionOk) {
        connectionOk = true;
        location.reload();
    }

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

function handleError() {
    console.log("Lost connection :'(");
    document.body.classList.add("w3-disabled");
    connectionOk = false;
}

(() => {
    loadStatus();
    setInterval(loadStatus, GLOBAL_TIMEOUT);
})();
