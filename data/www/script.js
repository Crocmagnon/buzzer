function play() {
    console.log("Play...");
    fetch("/play", { signal: AbortSignal.timeout(7000) });
}

function loadAvailableFiles() {
    console.log("Available files...");
    fetch("/available-files", { signal: AbortSignal.timeout(7000) })
        .then(response => response.json())
        .then(handleAvailableFiles);
}

function selectFile(name) {
    console.log("Select file");
    const body = new FormData();
    body.set("fileName", name);
    fetch("/select-file", { method: "POST", body: body, signal: AbortSignal.timeout(7000) })
        .then(response => response.json())
        .then(handleAvailableFiles);
}

function handleAvailableFiles(data) {
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
}

(() => {
    loadAvailableFiles();
    setInterval(loadAvailableFiles, 10000);
})();
