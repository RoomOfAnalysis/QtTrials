// https://openseadragon.github.io/docs/OpenSeadragon.html#.Options
let viewer = (window.openseadragon = OpenSeadragon({
    element: "openseadragon",
    prefixUrl: "images/",
    minZoomImageRatio: 0.01,
    visibilityRatio: 0,
    crossOriginPolicy: "Anonymous",
    ajaxWithCredentials: true,
    sequenceMode: true,
    showNavigator: true,
    showFullPageControl: false,
    showRotationControl: true,
    showFlipControl: true,
}));

viewer.scalebar({
    xOffset: 10,
    yOffset: 10,
    barThickness: 3,
    color: '#555555',
    fontColor: '#333333',
    backgroundColor: 'rgba(255, 255, 255, 0.5)',
    pixelsPerMeter: 1e6
});

viewer.innerTracker.keyHandler = null;

// Add a listener to the viewer to update the position and zoom info
var positionEl = document.querySelectorAll('.sidebar-right .position')[0];
var zoomEl = document.querySelectorAll('.sidebar-right .zoom')[0];

// Default values
positionEl.innerHTML = 'Web:<br> (NA) <br><br>Viewport:<br> (NA) <br><br>Image:<br> (NA)';
zoomEl.innerHTML = 'Zoom:<br> NA <br><br>Image Zoom:<br> NA';

var updateZoom = function () {
    var zoom = viewer.viewport.getZoom(true);
    var imageZoom = viewer.viewport.viewportToImageZoom(zoom);

    zoomEl.innerHTML = 'Zoom:<br>' + (Math.round(zoom * 100) / 100) +
        '<br><br>Image Zoom:<br>' + (Math.round(imageZoom * 100) / 100);
}

viewer.addHandler('open', function () {
    var tracker = new OpenSeadragon.MouseTracker({
        element: viewer.container,
        moveHandler: function (event) {
            var webPoint = event.position;
            var viewportPoint = viewer.viewport.pointFromPixel(webPoint);
            var imagePoint = viewer.viewport.viewportToImageCoordinates(viewportPoint);
            var zoom = viewer.viewport.getZoom(true);
            var imageZoom = viewer.viewport.viewportToImageZoom(zoom);

            positionEl.innerHTML = 'Web:<br>' + webPoint.toString() +
                '<br><br>Viewport:<br>' + viewportPoint.toString() +
                '<br><br>Image:<br>' + imagePoint.toString();

            updateZoom();
        }
    });
    tracker.setTracking(true);
    viewer.addHandler('animation', updateZoom);
});

document.getElementById("open-file").onclick = async (ev) => {
    try {
        const [fileHandle] = await window.showOpenFilePicker({
            multiple: false,
            types: [
                {
                    description: '',
                    accept: {
                        'image/*': ['.png', '.jpg', '.jpeg', '.tif', '.tiff', '.svs'],
                    }
                }
            ],
            startIn: 'downloads',
        });
        const file = await fileHandle.getFile();

        viewer.close();
        clearImageInfo();

        setupImage(file, file.name);
        document.getElementById("file-path").value = file.name;
    } catch (err) {
        if (err.name !== 'AbortError') {
            console.error('❌ Error:', err);
        }
    }
}

function setupImage(tileSourceInput, tilesourceName = "") {
    viewer.close();
    clearImageInfo();
    document.getElementById("filename").textContent = tilesourceName;

    // Check file type
    const isImage = tilesourceName.toLowerCase().match(/\.(png|jpg|jpeg)$/);
    const isTiff = tilesourceName.toLowerCase().match(/\.(tif|tiff|svs)$/);

    if (isImage) {
        // Handle PNG/JPG/JPEG files
        const url = URL.createObjectURL(tileSourceInput);
        viewer.addSimpleImage({
            url: url,
            success: function () {
                document.getElementById("filename").textContent += " -- Image loaded successfully";
                // Cleanup object URL after load
                URL.revokeObjectURL(url);
            },
            error: function (error) {
                document.getElementById("filename").textContent +=
                    ": Error opening image file. See console for details.";
                console.error(error);
                URL.revokeObjectURL(url);
            }
        });
    } else if (isTiff) {
        // Handle TIFF files
        let tiffTileSources = OpenSeadragon.GeoTIFFTileSource.getAllTileSources(tileSourceInput, {
            logLatency: true,
        });
        tiffTileSources.then((ts) => viewer.open(ts));

        tiffTileSources
            .then((tileSources) => {
                document.getElementById("filename").textContent +=
                    " -- " + tileSources.length + " image" + (tileSources.length != 1 ? "s" : "") + " found";
                Promise.all(tileSources.map((t) => t.promises.ready)).then(() =>
                    showTileSourcesInfo(tileSources)
                );
            })
            .catch((error) => {
                document.getElementById("filename").textContent +=
                    ": Error opening TIFF file. See console for details.";
                console.error(error);
            });
    } else {
        document.getElementById("filename").textContent +=
            ": Unsupported file format. Please use TIFF, PNG, JPG, or JPEG files.";
    }
}

function clearImageInfo() {
    document.getElementById("image-description").textContent = "";
    document.getElementById("associated-images").textContent = "";
}
function showTileSourcesInfo(tileSources) {
    clearImageInfo();
    let desc = document.getElementById("image-description");
    tileSources.map((ts, index) => {
        let images = ts.GeoTIFFImages;
        let h = document.createElement("h3");
        h.textContent = "TileSource #" + index;
        desc.appendChild(h);
        showImageInfo(images);
        desc.appendChild(document.createElement("hr"));
        return images;
    });
}

function showImageInfo(images) {
    let desc = document.getElementById("image-description");
    let frag = document.createDocumentFragment();

    images.forEach((image, index) => {
        let d = document.createElement("div");
        frag.appendChild(d);
        let t = document.createElement("h4");
        d.appendChild(t);
        t.textContent = "Tiff Page " + index;

        let fd = Object.assign({}, image.fileDirectory);
        if (fd.ImageDescription) {
            if (index == 0) {
                //console.log(fd.ImageDescription);
                let found = fd.ImageDescription.match(/MPP = (\d+([.]\d*)?([eE][+-]?\d+)?|[.]\d+([eE][+-]?\d+)?)/i);
                //console.log(found);
                if (found && found.length > 1)
                    viewer.scalebar({ pixelsPerMeter: (1e6 / parseFloat(found[1])) });
            }
            let info = document.createElement("div");
            d.appendChild(info);
            let ID =
                "<u>ImageDescription contents for this subimage</u><br>" +
                fd.ImageDescription.replaceAll("|", "<br>");
            delete fd.ImageDescription;
            info.innerHTML = ID;
        }

        let to_print = {};
        Object.entries(fd).forEach(([k, v]) => {
            to_print[k] =
                typeof v !== "string" && v.length > 8
                    ? "" + v.constructor.name + " (" + v.length + ") [...]"
                    : typeof v !== "string" && typeof v.length !== "undefined"
                        ? v.constructor.name + "(" + v.length + ") [" + [...v.values()] + "]"
                        : v;
        });

        let pre = document.createElement("pre");
        d.appendChild(pre);
        pre.textContent = JSON.stringify(to_print, null, 2);
    });
    desc.appendChild(frag);
}