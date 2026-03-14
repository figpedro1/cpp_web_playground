import { onMounted } from 'vue';

onMounted(() => {
    if (!document.getElementById('autodesk-viewer-script')) {
        const script = document.createElement('script');
        script.id = 'autodesk-viewer-script';
        script.src = "https://developer.api.autodesk.com/modelderivative/v2/viewers/viewer3D.min.js";
        document.head.appendChild(script);

        script.onload = () => {

        };
    }
})

<template>

</template>