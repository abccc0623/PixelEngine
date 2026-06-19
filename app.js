let feedback = [];
let selectedId = "";

const els = {
  updated: document.getElementById("updated"),
  stats: document.getElementById("stats"),
  search: document.getElementById("search"),
  area: document.getElementById("areaFilter"),
  status: document.getElementById("statusFilter"),
  list: document.getElementById("list"),
  detail: document.getElementById("detail"),
};

const statusLabels = ["전체", "미검토", "일감화", "폐기", "채택"];
const areaLabels = ["전체", "엔진", "에디터", "클라이언트", "그래픽", "물리", "사운드"];

function escapeHtml(value) {
  return String(value ?? "")
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;");
}

function statusClass(status) {
  if (status === "일감화") return "done";
  if (status === "폐기") return "drop";
  return "";
}

function populateFilters() {
  els.area.innerHTML = areaLabels.map((item) => `<option>${item}</option>`).join("");
  els.status.innerHTML = statusLabels.map((item) => `<option>${item}</option>`).join("");
}

function filtered() {
  const query = els.search.value.trim().toLowerCase();
  const area = els.area.value;
  const status = els.status.value;
  return feedback.filter((item) => {
    if (area !== "전체" && item.area !== area) return false;
    if (status !== "전체" && item.status !== status) return false;
    if (!query) return true;
    return [item.title, item.situation, item.problem, item.suggestion]
      .join(" ")
      .toLowerCase()
      .includes(query);
  });
}

function renderStats() {
  const counts = {
    전체: feedback.length,
    미검토: feedback.filter((item) => item.status === "미검토").length,
    일감화: feedback.filter((item) => item.status === "일감화").length,
    폐기: feedback.filter((item) => item.status === "폐기").length,
  };
  els.stats.innerHTML = Object.entries(counts)
    .map(([label, count]) => `<div class="stat"><span>${label}</span><strong>${count}</strong></div>`)
    .join("");
}

function renderList() {
  const items = filtered();
  if (!items.some((item) => item.path === selectedId)) selectedId = items[0]?.path || "";
  els.list.innerHTML = items.map((item) => `
    <button class="card ${item.path === selectedId ? "active" : ""}" data-id="${escapeHtml(item.path)}">
      <span class="card-title">${escapeHtml(item.title)}</span>
      <span class="chips">
        <span class="chip area">${escapeHtml(item.area)}</span>
        <span class="chip ${statusClass(item.status)}">${escapeHtml(item.status)}</span>
        <span class="chip">${escapeHtml(item.date)}</span>
      </span>
    </button>
  `).join("");
  document.querySelectorAll(".card").forEach((button) => {
    button.addEventListener("click", () => {
      selectedId = button.dataset.id;
      render();
    });
  });
}

function renderDetail() {
  const item = feedback.find((entry) => entry.path === selectedId);
  if (!item) {
    els.detail.innerHTML = `<div class="empty">피드백이 없습니다.</div>`;
    return;
  }
  els.detail.innerHTML = `
    <h2>${escapeHtml(item.title)}</h2>
    <div class="chips">
      <span class="chip area">${escapeHtml(item.area)}</span>
      <span class="chip ${statusClass(item.status)}">${escapeHtml(item.status)}</span>
      <span class="chip">${escapeHtml(item.date)}</span>
    </div>
    <div class="section"><h3>상황</h3><p>${escapeHtml(item.situation)}</p></div>
    <div class="section"><h3>문제</h3><p>${escapeHtml(item.problem)}</p></div>
    <div class="section"><h3>제안</h3><p>${escapeHtml(item.suggestion)}</p></div>
    ${item.discardReason ? `<div class="section"><h3>폐기 사유</h3><p>${escapeHtml(item.discardReason)}</p></div>` : ""}
  `;
}

function render() {
  renderStats();
  renderList();
  renderDetail();
}

async function load() {
  populateFilters();
  const response = await fetch("./feedback-data.json", { cache: "no-store" });
  const data = await response.json();
  feedback = data.feedback || [];
  els.updated.textContent = `Last updated: ${data.generatedAt || ""}`;
  render();
}

[els.search, els.area, els.status].forEach((element) => {
  element.addEventListener("input", render);
  element.addEventListener("change", render);
});

load().catch((error) => {
  els.detail.innerHTML = `<div class="empty">데이터를 불러오지 못했습니다: ${escapeHtml(error.message)}</div>`;
});
