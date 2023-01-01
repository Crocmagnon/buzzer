function play() {
    console.log("Play...");
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "play", true);
    xhttp.send();
}

function loadAvailableFiles() {
    console.log("Available files...");
    fetch("/available-files")
        .then(response => response.json())
        .then(handleAvailableFiles);
}

function selectFile(name) {
    console.log("Select file");
    const body = new FormData();
    body.set("fileName", name);
    fetch("/select-file", {method: "POST", body: body})
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
})();
