const botaoSelect = document.getElementById("botaoSelect");
botaoSelect.addEventListener ("click", click=>{
    const linha= document.getElementById("linhaOnibus").value;
    window.location.href="pages/listarMonitoramento.html?linha="+linha;
})
