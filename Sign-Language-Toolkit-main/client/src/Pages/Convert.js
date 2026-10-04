import React, { useEffect, useRef, useState } from "react";
import Slider from "react-input-slider";
import * as THREE from "three";
import { GLTFLoader } from "three/examples/jsm/loaders/GLTFLoader";
import { io } from "socket.io-client";

import "bootstrap/dist/css/bootstrap.min.css";
import "font-awesome/css/font-awesome.min.css";
import "../App.css";

import xbot from "../Models/xbot/xbot.glb";
import ybot from "../Models/ybot/ybot.glb";
import xbotPic from "../Models/xbot/xbot.png";
import ybotPic from "../Models/ybot/ybot.png";
import * as words from "../Animations/words";
import * as alphabets from "../Animations/alphabets";
import { defaultPose } from "../Animations/defaultPose";

function signTokens(value) {
  const inputWords = String(value || "").toUpperCase().match(/[A-Z]+/g) || [];
  const phrases = words.wordList
    .map(key => ({ key, parts: key.split("_") }))
    .sort((a, b) => b.parts.length - a.parts.length);
  const tokens = [];
  for (let index = 0; index < inputWords.length;) {
    const phrase = phrases.find(item =>
      item.parts.every((part, offset) => inputWords[index + offset] === part)
    );
    if (phrase) {
      tokens.push({ animation: phrase.key, label: phrase.parts.join(" ") });
      index += phrase.parts.length;
    } else {
      tokens.push({ animation: null, label: inputWords[index] });
      index += 1;
    }
  }
  return tokens;
}

function Convert({ navigate }) {
  const [translatedText, setTranslatedText] = useState("");
  const [inputText, setInputText] = useState("");
  const [bot, setBot] = useState(xbot);
  const [speed, setSpeed] = useState(0.1);
  const [pause, setPause] = useState(800);
  const [connected, setConnected] = useState(false);
  const [liveMessage, setLiveMessage] = useState(null);

  const engineRef = useRef({});
  const speedRef = useRef(speed);
  const pauseRef = useRef(pause);
  const seenMessageIds = useRef(new Set());

  useEffect(() => { speedRef.current = speed; }, [speed]);
  useEffect(() => { pauseRef.current = pause; }, [pause]);

  useEffect(() => {
    const ref = engineRef.current;
    const container = document.getElementById("canvas-container");
    let pauseTimer = null;

    ref.disposed = false;
    ref.flag = false;
    ref.pending = false;
    ref.animations = [];
    ref.characters = [];
    ref.waitingTexts = [];

    ref.scene = new THREE.Scene();
    ref.scene.background = new THREE.Color(0xfff6e5);
    ref.scene.add(new THREE.AmbientLight(0xffffff, 0.8));
    const spotLight = new THREE.SpotLight(0xffffff, 1);
    spotLight.position.set(0, 5, 5);
    ref.scene.add(spotLight);

    ref.renderer = new THREE.WebGLRenderer({ antialias: true });
    ref.renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2));
    ref.camera = new THREE.PerspectiveCamera(30, 1, 0.1, 1000);
    ref.camera.position.set(0, 1.4, 1.6);
    ref.camera.lookAt(0, 1.4, 0);
    container.innerHTML = "";
    container.appendChild(ref.renderer.domElement);

    const resize = () => {
      const width = Math.max(container.clientWidth, 320);
      const height = Math.max(container.clientHeight, 360);
      ref.camera.aspect = width / height;
      ref.camera.updateProjectionMatrix();
      ref.renderer.setSize(width, height, false);
      ref.renderer.render(ref.scene, ref.camera);
    };
    resize();
    window.addEventListener("resize", resize);

    ref.animate = () => {
      if (ref.disposed) return;
      if (ref.animations.length === 0) {
        ref.pending = false;
        ref.renderer.render(ref.scene, ref.camera);
        return;
      }

      if (ref.animations[0].length && !ref.flag) {
        if (ref.animations[0][0] === "start-message") {
          setTranslatedText("");
          ref.animations.shift();
        } else if (ref.animations[0][0] === "add-text") {
          setTranslatedText(previous => previous + ref.animations[0][1]);
          ref.animations.shift();
        } else if (ref.avatar) {
          for (let i = 0; i < ref.animations[0].length;) {
            const [boneName, action, axis, limit, sign] = ref.animations[0][i];
            const bone = ref.avatar.getObjectByName(boneName);
            if (!bone) {
              ref.animations[0].splice(i, 1);
            } else if (sign === "+" && bone[action][axis] < limit) {
              bone[action][axis] = Math.min(bone[action][axis] + speedRef.current, limit);
              i += 1;
            } else if (sign === "-" && bone[action][axis] > limit) {
              bone[action][axis] = Math.max(bone[action][axis] - speedRef.current, limit);
              i += 1;
            } else {
              ref.animations[0].splice(i, 1);
            }
          }
        }
      } else if (ref.animations[0].length === 0) {
        ref.flag = true;
        pauseTimer = window.setTimeout(() => { ref.flag = false; }, pauseRef.current);
        ref.animations.shift();
      }

      ref.renderer.render(ref.scene, ref.camera);
      ref.animationFrame = requestAnimationFrame(ref.animate);
    };

    ref.enqueueText = (value) => {
      const cleanText = String(value || "").trim();
      if (!cleanText) return;
      if (!ref.avatar) {
        ref.waitingTexts.push(cleanText);
        return;
      }

      const wasPending = ref.pending;
      ref.pending = true;
      ref.animations.push(["start-message"]);
      for (const token of signTokens(cleanText)) {
        if (token.animation && words[token.animation]) {
          ref.animations.push(["add-text", `${token.label} `]);
          words[token.animation](ref);
        } else {
          [...token.label].forEach((character, index) => {
            if (!alphabets[character]) return;
            ref.animations.push(["add-text", index === token.label.length - 1 ? `${character} ` : character]);
            alphabets[character](ref);
          });
        }
      }
      if (!wasPending) ref.animate();
    };

    const loader = new GLTFLoader();
    loader.load(bot, (gltf) => {
      if (ref.disposed) return;
      gltf.scene.traverse(child => {
        if (child.type === "SkinnedMesh") child.frustumCulled = false;
      });
      ref.avatar = gltf.scene;
      ref.scene.add(ref.avatar);
      defaultPose(ref);
      const waiting = ref.waitingTexts.splice(0);
      waiting.forEach(value => ref.enqueueText(value));
      resize();
    });

    return () => {
      ref.disposed = true;
      window.removeEventListener("resize", resize);
      window.clearTimeout(pauseTimer);
      cancelAnimationFrame(ref.animationFrame);
      ref.renderer.dispose();
      container.innerHTML = "";
    };
  }, [bot]);

  useEffect(() => {
    const socket = io({ path: "/socket.io" });
    const playMessage = (message) => {
      if (!message || !message.text || seenMessageIds.current.has(message.id)) return;
      seenMessageIds.current.add(message.id);
      setLiveMessage(message);
      engineRef.current.enqueueText(message.text);
    };

    socket.on("connect", () => {
      setConnected(true);
      socket.emit("get_sign_state");
    });
    socket.on("disconnect", () => setConnected(false));
    socket.on("sign_message", playMessage);
    socket.on("sign_state", data => playMessage(data && data.message));
    return () => socket.disconnect();
  }, []);

  const translateManualText = () => {
    const text = inputText.trim();
    if (!text) return;
    setLiveMessage({ sender_name: "Manual preview", text });
    engineRef.current.enqueueText(text);
    setInputText("");
  };

  return (
    <main className="sign-display-shell">
      <header className="sign-display-header">
        <div>
          <h1>ABILITY SIGN DISPLAY</h1>
          <p>Incoming text for the deaf/glove user is animated here automatically.</p>
        </div>
        <div className={`sign-connection ${connected ? "online" : "offline"}`}>
          {connected ? "● CHAT SYNC ONLINE" : "● CHAT SYNC OFFLINE"}
        </div>
      </header>

      <section className="sign-display-grid">
        <aside className="sign-control-panel">
          <div className="live-message-card">
            <span>NOW SIGNING</span>
            <strong>{liveMessage ? liveMessage.text : "Waiting for an incoming message…"}</strong>
            <small>{liveMessage ? `From: ${liveMessage.sender_name}` : "Open Ability Chat on the other display."}</small>
          </div>

          <label>Animation progress</label>
          <div className="translated-output">{translatedText || "—"}</div>

          <label htmlFor="manualText">Manual preview</label>
          <textarea id="manualText" rows={3} value={inputText} onChange={event => setInputText(event.target.value)} placeholder="Type text to preview…" />
          <button className="sign-action-button" onClick={translateManualText}>Translate text</button>

          <label>Speed: {speed.toFixed(2)}</label>
          <Slider axis="x" xmin={0.05} xmax={0.5} xstep={0.01} x={speed} onChange={({ x }) => setSpeed(x)} className="w-100" />
          <label>Pause: {pause} ms</label>
          <Slider axis="x" xmin={0} xmax={2000} xstep={100} x={pause} onChange={({ x }) => setPause(x)} className="w-100" />

          <button className="sign-secondary-button" onClick={() => navigate("learn")}>Open learning mode</button>
        </aside>

        <div id="canvas-container" className="sign-avatar-stage" aria-label="Animated sign-language avatar" />

        <aside className="avatar-picker">
          <h2>Avatar</h2>
          <button className={bot === xbot ? "selected" : ""} onClick={() => setBot(xbot)}><img src={xbotPic} alt="Select XBOT" /></button>
          <button className={bot === ybot ? "selected" : ""} onClick={() => setBot(ybot)}><img src={ybotPic} alt="Select YBOT" /></button>
          <p>Known words use full signs. Other A–Z text is fingerspelled.</p>
        </aside>
      </section>
    </main>
  );
}

export default Convert;
