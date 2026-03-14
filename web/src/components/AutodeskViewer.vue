<script setup lang="ts">
import { onMounted } from 'vue';

const loadScript = (src: string, id: string) => {
    return new Promise((resolve, reject) => {
        const script = document.createElement('script');
        script.src = src;
        script.id = id;
        script.onload = resolve;
        script.onerror = reject;
        document.head.appendChild(script);
    })
};

onMounted(async () => {
    try {
        if(!document.querySelector('#autodesk-viewer-stylesheet')){
            const link = document.createElement('link');
            link.rel = 'stylesheet';
            link.href = 'https://developer.api.autodesk.com/modelderivative/v2/viewers/7.*/style.min.css';
            link.type = 'text/css';
            link.id = 'autodesk-viewer-stylesheet'
            document.head.appendChild(link);
            
            console.log("autodesk-viewer-stylesheet carregado com sucesso!")
        }
        
        if(!document.querySelector('#autodesk-viewer-script')){
            await loadScript('https://developer.api.autodesk.com/modelderivative/v2/viewers/7.*/viewer3D.min.js', 'autodesk-viewer-script');
            console.log("autodesk-viewer-script carregado com sucesso!");
        };

        console.log("Autodesk viewer pronto para uso!");

    } catch (err) {
        console.error("Erro ao carregar os scripts Autodesk: ", err);
    }
})


</script>

<template>
    <div class="test">
        <div id="autodesk-viewer"></div>
    </div>
</template>

<style scoped>
        #autodesk-viewer {
            width: 100%;
            height: 100%;
            margin: 0;
            background-color: #F0F8FF;
            border: 1px solid black;
        }
        .test {
            height: 500px;
        }
</style>