const GLOBAL_TIMEOUT = 10000;
let connectionOk = true;
let statusTimeout = null;
let selectedFile = "";

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

async function loadStatus() {
    console.log("Status...");
    return fetch("/status", { signal: AbortSignal.timeout(GLOBAL_TIMEOUT) })
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

    if (data.files.selected != selectedFile) {
        selectedFile = data.files.selected;
        document.querySelectorAll(".w3-green").forEach(element => {
            element.classList.remove("w3-green");
            element.classList.add("w3-blue");
        });
        const previouslySelected = document.querySelector(`[data-name='${selectedFile}']`);
        if (previouslySelected) {
            previouslySelected.classList.remove("w3-blue");
            previouslySelected.classList.add("w3-green");
        }
    }

    document.getElementById("volume-current").innerText = data.volume.current;
    document.getElementById("volume-increase").disabled = !data.volume.canIncrease;
    document.getElementById("volume-decrease").disabled = !data.volume.canDecrease;
    statusTimeout = setTimeout(loadStatus, GLOBAL_TIMEOUT);
}

async function listFiles(cursor=0) {
    console.log("List files...");
    return fetch(`/list-files?cursor=${cursor}`, { signal: AbortSignal.timeout(GLOBAL_TIMEOUT) })
        .then(response => response.json())
        .then(data => {
            let dom = "";
            data.files.forEach(element => {
                if (!element) {
                    // Filter out null
                    return;
                }
                let className = "w3-blue";
                if (element === selectedFile) {
                    className = "w3-green";
                }
                dom += `<button class="w3-button ${className}" data-name="${element}" onclick="selectFile('${element}')">${element}</button>`;
            });
            if (data.files.moreNotShown) {
                dom += `<button class="w3-button w3-gray" disabled="" title="D'autres fichiers sont disponibles">...</button>`;
            }
            const availableFilesNode = document.getElementById("available-files");
            if (cursor == 0) {
                availableFilesNode.innerHTML = dom;
            }
            else {
                availableFilesNode.innerHTML += dom;
            }
            if (data.next) {
                return listFiles(data.next);
            }
        })
        .catch(handleError);
}

function handleError(error) {
    console.error(error);
    clearTimeout(statusTimeout);
    console.log("Lost connection :'(");
    document.body.classList.add("w3-disabled");
    connectionOk = false;
    statusTimeout = setTimeout(loadStatus, GLOBAL_TIMEOUT);
}

(() => {
    loadStatus().then(() => listFiles());
    statusTimeout = setTimeout(loadStatus, GLOBAL_TIMEOUT);
})();
