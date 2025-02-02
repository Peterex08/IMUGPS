import { useState, useEffect, Suspense } from "react";
import { useSpring } from "@react-spring/three";

import * as THREE from 'three';
import { Environment, OrbitControls } from '@react-three/drei';
import { Canvas } from '@react-three/fiber';
import Rocket from '/public/rocket_model/Rocket';
import Plot from "react-plotly.js";
import mqtt from "mqtt";

const MQTT_BROKER = "wss://broker.hivemq.com:8884/mqtt";
const MQTT_TOPIC = "epta_strean/payloadata223";

function Home() {

  const [latestData, setLatestData] = useState(null);
  const [history, setHistory] = useState([]);

  const [payload, setPayload] = useState([]);
  const [attitude, setAttitude] = useState([-1.571, -0.005, 3.066]);
  const [connected, setConnected] = useState(false);

  const [points, setPoints] = useState({
    lat: [],
    lon: [],
    alt: [],
  });

  const addRandomPoint = (x, y, z) => {
    setPoints((prevPoints) => ({
      lat: [...prevPoints.lat, x], // Adiciona um valor aleatório em X
      lon: [...prevPoints.lon, y], // Adiciona um valor aleatório em Y
      alt: [...prevPoints.alt, z], // Adiciona um valor aleatório em Z
    }));
  };

  useEffect(() => {
    const client = mqtt.connect(MQTT_BROKER);

    client.on("connect", () => {
      console.log("Conectado ao MQTT");
      client.subscribe(MQTT_TOPIC);
    });

    client.on("message", (topic, message) => {
      const decodedData = decodePayload(message);
      if (decodedData) {
        setLatestData(decodedData);
        setHistory((prevHistory) => [...prevHistory, decodedData]);
        setAttitude([decodedData.roll*Math.PI/180, decodedData.pitch*Math.PI/180, decodedData.yaw*Math.PI/180]);
        addRandomPoint(decodedData.lat, decodedData.lon, decodedData.alt);
      }
    });

    return () => {
      client.end();
      
    };
  }, []);

  useEffect(() => {
    const checkIfEmpty =() => {
      if(Array.isArray(attitude) && attitude.length === 0) {
        setConnected(false);
      } else {
        setConnected(true);
      }
    };
    checkIfEmpty();
  }, [attitude]);

  const { rotation } = useSpring({
    rotation: attitude,
    config: { tension: 200, friction: 30 }, // Ajuste de suavidade
  });

  const resetPoints = () => {
    setPoints({ lat: [], lon: [], alt: [] });
  };

  return (
    <div className="bg-gradient-to-t from-[#23395d] to-[#152238] w-screen h-screen">
      <div className="flex bg-[rgba(242,242,242,0.4)] h-[6%] items-center justify-between px-2"> {/* Navbar */}
        <img src="/eptalogobr.png" className="h-16 my-auto"/> {/* Logo */}
        <p className="font-montserrat font-bold text-[#f2f2f2]">Telemetry Streaming</p> {/* Botão Logout */}
        <a
          className="flex flex-row gap-x-1 cursor-pointer bg-[#23395d] hover:bg-[#152238] active:bg-[#152238] p-2 rounded-xl w-fit h-fit"
          href="/logout"
          >
          <img src="/logout.svg" className="w-5"/>
          <p className="font-montserrat font-semibold text-[#f2f2f2]">Logout</p>        
        </a>
      </div>
      <div className="grid grid-cols-3 grid-rows-4 gap-4 m-4"> {/* Grid */}
        <div className="row-span-4 col-span-2">
          <RocketViewer rot={rotation}/>
        </div>
        <Stts att={connected} className=""/>
        <Paraquedas status={false} conn={connected} className=""/>
        <VelD veld={15} conn={connected} className=""/>
        <Altttd alt={80} conn={connected} className=""/>
        
      </div>
      <div className="grid grid-cols-3 grid-rows-5 gap-4 m-4"> {/* Grid */}
        <div className="row-span-4 col-span-3">
          <Graph points={points} className=""/>
        </div>
        <div className="col-span-2">
          <button
            className="flex flex-row gap-x-1 justify-center items-center cursor-pointer disabled:bg-[#A7A2A9] bg-[#42809b] hover:bg-[#152238] active:bg-[#152238] p-2 rounded-xl w-full h-[50%]"
            onClick={resetPoints}
            disabled={!connected}
            >
            <img src="/save.svg" className="w-5"/>
            <p className="font-montserrat font-semibold text-center text-[#f2f2f2]">Capturar Trajetória</p>        
          </button>
        </div>
        <div>
          <button
            className="flex flex-row gap-x-1 justify-center items-center cursor-pointer disabled:bg-[#A7A2A9] bg-[#b22222] hover:bg-[#832f2f] active:bg-[#832f2f] p-2 rounded-xl w-full h-[50%]"
            onClick={resetPoints}
            disabled={!connected}
            >
            <img src="/reset.svg" className="w-5"/>
            <p className="font-montserrat font-semibold text-center text-[#f2f2f2]">Reset</p>        
          </button>
        </div>
      </div>      
    </div>
  );
}

export default Home;

const RocketViewer = (rot) => {

  return(
    <div className="relative rounded-xl bg-[rgba(242,242,242,0.3)] h-full"> {/* Rocket Viewer */}
      <div className="absolute top-0 left-0 w-full mt-1"> {/* Status */}
        <p className="font-montserrat font-medium text-[11px] text-[#f2f2f2] px-2">Atitude do Foguete:</p>
        <hr className="border-t-1 mx-2 border-[#f2f2f2] opacity-30"/>
      </div>
      <Canvas className="h-full">
        <ambientLight/>
        <Suspense fallback={null}>
          <Rocket rota={rot}/>
        </Suspense>
        <Environment preset="city" />
      </Canvas>
    </div>
  )
};

const Stts = ({ att }) => {

  return(
    <div className={`flex flex-col w-full rounded-xl py-1 ${att ? "bg-[#337268]" : "bg-[#b22222]"}`}>
      <p className="font-montserrat font-medium text-[10px] text-[#f2f2f2] px-2">Status do Foguete:</p>
      <hr className="border-t-1 mx-2 border-[#f2f2f2] opacity-30"/>
      {att &&
      <p className="font-montserrat font-bold text-2xl text-[#f2f2f2] px-2">Online</p>
      ||
      <p className="font-montserrat font-bold text-2xl text-[#f2f2f2] px-2">Offline</p>
      }
    </div>
  )
};

const VelD = ({ veld, conn }) => {

  return(
    <div className={`flex flex-col w-full rounded-xl py-1 bg-[#23395d]`}>
      <p className="font-montserrat font-medium text-[10px] text-[#f2f2f2] px-2">Velocidade Vertical:</p>
      <hr className="border-t-1 mx-2 border-[#f2f2f2] opacity-30"/>      
      {conn &&
      <>
        <div className="flex items-center gap-1">
          <p className="font-montserrat font-bold text-2xl text-[#f2f2f2] pl-2">{veld}</p>
          <p className="font-montserrat font-bold text-xl text-[#f2f2f2]">m/s</p>
        </div>
      </>
      ||
        <p className="font-montserrat font-bold text-2xl text-center text-[#f2f2f2] px-2">---</p>
      }   
    </div>
  )
};

const Altttd = ({ alt, conn }) => {
  
  return(
    <div className={`flex flex-col w-full rounded-xl py-1 bg-[#23395d]`}>
      <p className="font-montserrat font-medium text-[10px] text-[#f2f2f2] px-2">Altitude:</p>
      <hr className="border-t-1 mx-2 border-[#f2f2f2] opacity-30"/>
      {conn &&
      <>
        <div className="flex items-center gap-1">
          <p className="font-montserrat font-bold text-2xl text-[#f2f2f2] pl-2">{alt}</p>
          <p className="font-montserrat font-bold text-xl text-[#f2f2f2]">m</p>
        </div>
      </>
      ||
        <p className="font-montserrat font-bold text-2xl text-center text-[#f2f2f2] px-2">---</p>
      }   
    </div>
  )
};

const Paraquedas = ({ status, conn }) => {
  
  return(
    <div className={`flex flex-col w-full rounded-xl py-1 ${conn ? (!status ? "bg-[#ff6c2f]" : "bg-[#337268]") : "bg-[#23395d]"}`}>
      <p className="font-montserrat font-medium text-[10px] text-[#f2f2f2] px-2">Paraquedas:</p>
      <hr className="border-t-1 mx-2 border-[#f2f2f2] opacity-30"/>
      {conn &&
      <>
        {!status &&
        <p className="font-montserrat font-bold text-2xl text-[#f2f2f2] px-2">Armado</p>
        ||
        <p className="font-montserrat font-bold text-2xl text-[#f2f2f2] px-2">Acionado</p>
        }
      </>
      ||
        <p className="font-montserrat font-bold text-2xl text-center text-[#f2f2f2] px-2">---</p>
      }      
    </div>
  )
};

const Graph = ({ points }) => {

  return(
    <div className="h-fit rounded-xl bg-[rgba(242,242,242,0.3)]">
      <p className="font-montserrat pt-1 font-medium text-[11px] text-[#f2f2f2] px-2">Trajetória do Foguete:</p>
      <hr className="border-t-1 mx-2 border-[#f2f2f2] opacity-30"/>
      <Plot
        data={[
          {
            x: points.lat,
            y: points.lon,
            z: points.alt,
            type: "scatter3d",
            mode: "markers",
            marker: { size: 5, color: "#6CD4FF" },
          },
        ]}
        layout={{
          title: "Pontos Atualizando em Tempo Real",
          autosize: true,
          scene: {
            xaxis: { 
              title: "Lat",
              titlefont: { color: "white" }, // Cor da legenda do eixo X
              tickfont: { color: "white" },  // Cor dos números do eixo X
              gridwidth: 2,               // Espessura das linhas da malha no eixo X

            },
            yaxis: { 
              title: "Lon",
              titlefont: { color: "white" }, // Cor da legenda do eixo X
              tickfont: { color: "white" },  // Cor dos números do eixo X
              gridwidth: 2,
            },
            zaxis: {
              title: "Alt",
              titlefont: { color: "white" }, // Cor da legenda do eixo X
              tickfont: { color: "white" },  // Cor dos números do eixo X
              gridwidth: 2,
            },
            bgcolor: "rgba(0,0,0,0)",
            camera: { eye: { x: 1.5, y: 1.5, z: 1.5 } },
          },
          paper_bgcolor: "rgba(0,0,0,0)",
          margin: { l: 0, r: 0, t: 0, b: 0 },
        }}
        config={{
          displayModeBar: false,
        }}
        style={{ width: "100%", height: "400px" }}
      />
    </div>
  )
};

function decodePayload(buffer) {
  if (buffer.length !== 48) {
    console.error("Tamanho incorreto do buffer");
    return null;
  }

  const view = new DataView(buffer.buffer, buffer.byteOffset, buffer.length);

  try {
    return {
      time: view.getFloat32(0, true),
      lat: view.getFloat32(4, true),
      lon: view.getFloat32(8, true),
      alt: view.getFloat32(12, true),
      veld: view.getFloat32(16, true),
      veln: view.getFloat32(20, true),
      vele: view.getFloat32(24, true),
      yaw: view.getFloat32(28, true),
      pitch: view.getFloat32(32, true),
      roll: view.getFloat32(36, true),
      bar: view.getFloat32(40, true),
      parachute: view.getFloat32(44, true),
    };
  } catch (error) {
    console.error("Erro ao decodificar:", error);
    return null;
  }
}