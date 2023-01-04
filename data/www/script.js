const GLOBAL_TIMEOUT = 10000;
let connectionOk = true;
let statusTimeout = null;

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
    clearTimeout(statusTimeout);
    document.body.classList.remove("w3-disabled");
    if (!connectionOk) {
        connectionOk = true;
        location.reload();
    }

    let dom = "";
    data.files.available.forEach((element, index) => {
        let className = "w3-blue";
        if (index === data.files.selectedIndex) {
            className = "w3-green";
        }
        dom += `<button class="w3-button ${className}" onclick="selectFile('${element}')">${element}</button>`;
    });
    if (data.files.moreNotShown) {
        dom += `<button class="w3-button w3-gray" disabled="" title="D'autres fichiers sont disponibles">...</button>`;
    }
    document.getElementById("available-files").innerHTML = dom;

    document.getElementById("volume-current").innerText = data.volume.current;
    document.getElementById("volume-increase").disabled = !data.volume.canIncrease;
    document.getElementById("volume-decrease").disabled = !data.volume.canDecrease;
    statusTimeout = setTimeout(loadStatus, GLOBAL_TIMEOUT);
}

function handleError() {
    clearTimeout(statusTimeout);
    console.log("Lost connection :'(");
    document.body.classList.add("w3-disabled");
    connectionOk = false;
    statusTimeout = setTimeout(loadStatus, GLOBAL_TIMEOUT);
}

(() => {
    loadStatus();
    statusTimeout = setTimeout(loadStatus, GLOBAL_TIMEOUT);
})();
