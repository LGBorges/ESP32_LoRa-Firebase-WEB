// Import the functions you need from the SDKs you need
import {
  initializeApp
} from "https://www.gstatic.com/firebasejs/9.1.2/firebase-app.js";

import "https://code.jquery.com/jquery-3.6.0.js"

import {
  getDatabase,
  ref,
  get,
  set,
  child,
  update,
  remove
} from "https://www.gstatic.com/firebasejs/9.1.2/firebase-database.js"
// TODO: Add SDKs for Firebase products that you want to use
// https://firebase.google.com/docs/web/setup#available-libraries

// Your web app's Firebase configuration
window.onload = inicializar

const firebaseConfig = {
  apiKey: "",
  authDomain: "",
  projectId: "",
  storageBucket: "",
  messagingSenderId: "",
  appId: ""
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);


const db = getDatabase();
const temperatura = document.getElementById('temperatura');
const numeroPassageiros = document.getElementById('numeroPassageiros');
const umidade = document.getElementById('umidade');
const maximoPassageiros = document.getElementById('maximoPassageiros');
const nomeLinha = document.getElementById('linhaNome')
const dbref = ref(db);

function buscarLinha(linha) {
  get(child(dbref, "" + linha + "")).then((snapshot) => {
    if (snapshot.exists()) {
      temperatura.innerHTML = snapshot.val().temperatura + "ºC";
      numeroPassageiros.innerHTML = snapshot.val().numeroPassageiros;
      maximoPassageiros.innerHTML = snapshot.val().maximoPassageiros;
      umidade.innerHTML = snapshot.val().umidade + "%";
      //IF PROVISORIO
      if (linha === "Linha_501_3/") {
        nomeLinha.innerHTML = "Linha 501.3";
      } else {
        nomeLinha.innerHTML = "Linha 501.4";
      }
    } else {
      alert("A linha não foi achada.");
    }
  }).catch((error) => {
    alert("erro ocorrido, error", +error);
  });
}



function inicializar() {
  const url = new URLSearchParams(window.location.search);
  const linha = url.get('linha');

  if (linha != null) {
    buscarLinha(linha);
  }
}

setInterval(function () {
  $(document).ready(function () {
      const url = new URLSearchParams(window.location.search);
      const linha = url.get('linha');
    get(child(dbref, "" + linha + "")).then((snapshot) => {
      if (snapshot.exists()) {
        const temperaturaVariavel = snapshot.val().temperatura + "ºC";
        const numeroPassageirosVariavel = snapshot.val().numeroPassageiros;
        const umidadeVariavel = snapshot.val().umidade + "%";        
        if(temperatura.innerHTML != temperaturaVariavel || numeroPassageiros.innerHTML != numeroPassageirosVariavel || umidade.innerHTML != umidadeVariavel ){      
          const dt = new Date();
          const dtMonth = dt.getMonth() +1;
          const dataAtualizacao= "Última Atualização: " +dt.getDay()+ "/"+dtMonth+ "/"+dt.getFullYear()+ "  "+dt.getHours() + ":" + dt.getMinutes()+ ":" + dt.getSeconds();
          $("#dataAtualizacao").text(dataAtualizacao);          
        }        
        $("#temperatura").text(temperaturaVariavel);
        $("#numeroPassageiros").text(numeroPassageirosVariavel);
        $("#umidade").text(umidadeVariavel);
      }
    })
  })
}, 5000);
