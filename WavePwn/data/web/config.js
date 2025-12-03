/*
  config.js - WavePwn v2
  Web Config intuitiva (Bootstrap 5) + validação + integração completa
  com as rotas existentes do firmware.

  Endpoints usados:
    - GET/POST /api/config/device
    - GET       /api/lab/status
    - POST      /api/lab/set_pin
    - POST      /api/lab/unlock
    - POST      /api/gemini/key
    - POST      /api/gemini/ask   (para teste da chave)
    - GET       /reboot
*/

function $(id) {
  return document.getElementById(id);
}

function show_alert(containerId, type, message) {
  const container = $(containerId);
  if (!container) return;
  if (!message) {
    container.innerHTML = "";
    return;
  }
  const cls = type === "success" ? "alert-success"
              : type === "warning" ? "alert-warning"
              : type === "danger" ? "alert-danger"
              : "alert-info";
  container.innerHTML =
    '<div class="alert ' + cls + ' mb-0" role="alert">' +
    message +
    "</div>";
}

function update_neura9_label(v) {
  const label = $("neura9_sens_label");
  if (!label) return;
  const value = Number(v || $("neura9_sens")?.value || 0.78);
  const clamped = Math.min(1.0, Math.max(0.1, value));
  label.textContent =
    clamped.toFixed(2) +
    " · sensibilidade da NEURA9 (0.10 = conservadora · 1.00 = agressiva)";
}

function is_valid_lab_pin(pin) {
  return /^[0-9]{6}$/.test(pin || "");
}

// -----------------------------------------------------------------------------
// Carregamento inicial
// -----------------------------------------------------------------------------

async function load_device_config() {
  try {
    const res = await fetch("/api/config/device");
    if (!res.ok) {
      console.warn("[CFG] Falha ao carregar device_config.json:", res.status);
      return;
    }
    const cfg = await res.json();

    const deviceName = cfg.device_name || "WavePwn";
    const owner = cfg.owner || "";
    const language = cfg.language || cfg.lang || "pt-BR";
    const theme = cfg.theme || "dark";
    const assistant = cfg.assistant || "none";
    const enable_voice_alerts =
      typeof cfg.enable_voice_alerts === "boolean"
        ? String(cfg.enable_voice_alerts)
        : "true";
    const sens =
      typeof cfg.neura9_sensitivity === "number"
        ? cfg.neura9_sensitivity
        : 0.78;

    if ($("device_name")) $("device_name").value = deviceName;
    if ($("owner")) $("owner").value = owner;
    if ($("lang")) $("lang").value = language;
    if ($("theme")) $("theme").value = theme;
    if ($("assistant")) $("assistant").value = assistant;
    if ($("voice_alerts")) $("voice_alerts").value = enable_voice_alerts;
    if ($("neura9_sens")) {
      $("neura9_sens").value = sens;
      update_neura9_label(sens);
    }
  } catch (e) {
    console.error("[CFG] Erro ao carregar config:", e);
  }
}

async function refresh_lab_status() {
  try {
    const res = await fetch("/api/lab/status");
    if (!res.ok) {
      show_alert(
        "lab_status",
        "warning",
        "Falha ao consultar status do Lab Mode (" + res.status + ")."
      );
      return;
    }
    const st = await res.json();

    const guard = !!st.lab_guard_file;
    const pinSet = !!st.pin_set;
    const unlocked = !!st.lab_unlocked;

    let msg = "";
    if (guard && pinSet && unlocked) {
      msg =
        "Lab Mode ATIVO para esta sessão. Logs de simulação em /sd/lab_logs.";
      show_alert("lab_status", "success", msg);
    } else {
      const parts = [];
      if (!guard) parts.push("arquivo /sd/.enable_lab_attacks ausente");
      if (!pinSet) parts.push("PIN ainda não definido");
      if (!unlocked) parts.push("sessão atual ainda não desbloqueada");
      msg =
        "Lab Mode ainda bloqueado: " +
        (parts.length ? parts.join(" · ") : "verifique configuração.");
      show_alert("lab_status", "info", msg);
    }
  } catch (e) {
    console.error("[LAB] Erro ao consultar status:", e);
    show_alert(
      "lab_status",
      "danger",
      "Erro ao consultar status do Lab Mode."
    );
  }
}

// -----------------------------------------------------------------------------
// Ações
// -----------------------------------------------------------------------------

async function save_device_section() {
  const device_name =
    ($("device_name")?.value || "").trim() || "WavePwn";
  const owner = ($("owner")?.value || "").trim();
  const language = $("lang")?.value || "pt-BR";
  const theme = $("theme")?.value || "dark";
  const assistant = $("assistant")?.value || "none";
  const voice_alerts = $("voice_alerts")?.value === "true";
  const neura9_sensitivity =
    Number($("neura9_sens")?.value) || 0.78;

  const body = {
    device_name,
    owner,
    language,
    theme,
    assistant,
    enable_voice_alerts: voice_alerts,
    neura9_sensitivity: neura9_sensitivity,
  };

  const res = await fetch("/api/config/device", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(body),
  });

  if (!res.ok) {
    throw new Error("HTTP " + res.status);
  }
}

async function enable_lab() {
  const pin = ($("lab_pin")?.value || "").trim();
  if (!is_valid_lab_pin(pin)) {
    show_alert(
      "lab_status",
      "warning",
      "PIN inválido. Use exatamente 6 dígitos."
    );
    return;
  }

  try {
    // 1) Salva/atualiza PIN
    let res = await fetch("/api/lab/set_pin", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ pin }),
    });
    if (!res.ok) {
      show_alert(
        "lab_status",
        "danger",
        "Falha ao salvar PIN (" + res.status + ")."
      );
      return;
    }

    // 2) Desbloqueia sessão atual
    res = await fetch("/api/lab/unlock", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ pin }),
    });

    const j = await res.json().catch(() => ({}));
    if (!res.ok || !j.ok) {
      show_alert(
        "lab_status",
        "danger",
        "PIN salvo, mas desbloqueio falhou: " +
          (j.error || res.status)
      );
      return;
    }

    $("lab_pin").value = "";
    show_alert(
      "lab_status",
      "success",
      "Lab Mode desbloqueado para esta sessão (PIN salvo em /config/lab_config.json)."
    );
  } catch (e) {
    console.error("[LAB] Erro em enable_lab:", e);
    show_alert("lab_status", "danger", "Erro ao ativar Lab Mode.");
  }
}

async function test_gemini() {
  const key = ($("gemini_key")?.value || "").trim();

  show_alert(
    "gemini_status",
    "info",
    "Testando chave e conectividade com o Gemini..."
  );

  try {
    // Se o usuário digitou uma chave nova, salva antes de testar.
    if (key) {
      const resKey = await fetch("/api/gemini/key", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ key }),
      });
      if (!resKey.ok) {
        show_alert(
          "gemini_status",
          "danger",
          "Falha ao salvar chave (" + resKey.status + ")."
        );
        return;
      }
    }

    const prompt =
      "Responda apenas com a palavra OK. Este é um teste de conexão do WavePwn v2.";

    const res = await fetch("/api/gemini/ask", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ prompt }),
    });

    if (!res.ok) {
      show_alert(
        "gemini_status",
        "danger",
        "Falha ao chamar Gemini (" + res.status + ")."
      );
      return;
    }

    const j = await res.json().catch(() => ({}));
    const text = (j && j.response) || "(sem resposta)";

    if (text.toUpperCase().indexOf("OK") !== -1) {
      show_alert(
        "gemini_status",
        "success",
        "Conexão com Gemini OK. A chave parece válida."
      );
    } else {
      show_alert(
        "gemini_status",
        "warning",
        "Resposta recebida, mas não pôde ser validada como OK: " + text
      );
    }
  } catch (e) {
    console.error("[Gemini] Erro em test_gemini:", e);
    show_alert(
      "gemini_status",
      "danger",
      "Erro ao testar conexão com o Gemini."
    );
  }
}

async function save_all() {
  try {
    // 1) Configuração principal
    await save_device_section();

    // 2) Lab Mode (se houver PIN informado)
    const labPin = ($("lab_pin")?.value || "").trim();
    if (labPin) {
      if (!is_valid_lab_pin(labPin)) {
        show_alert(
          "lab_status",
          "warning",
          "PIN inválido. Use exatamente 6 dígitos."
        );
        return;
      }
      await enable_lab();
    }

    // 3) Chave Gemini (se houver nova chave informada, mas sem teste explícito)
    const key = ($("gemini_key")?.value || "").trim();
    if (key) {
      const resKey = await fetch("/api/gemini/key", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ key }),
      });
      if (!resKey.ok) {
        show_alert(
          "gemini_status",
          "danger",
          "Falha ao salvar chave Gemini (" + resKey.status + ")."
        );
        return;
      }
    }

    alert("Configuração salva! Reiniciando o WavePwn...");
    // Endpoint simples para reboot remoto
    window.location.href = "/reboot";
  } catch (e) {
    console.error("[CFG] Erro em save_all:", e);
    alert("Erro ao salvar configuração. Veja o console para detalhes.");
  }
}

// -----------------------------------------------------------------------------
// Bootstrap inicial
// -----------------------------------------------------------------------------

document.addEventListener("DOMContentLoaded", () => {
  load_device_config();
  refresh_lab_status();
  update_neura9_label();
});