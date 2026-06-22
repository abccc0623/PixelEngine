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

const ALL = "전체";
const STATUS_UNREVIEWED = "미검토";
const STATUS_DONE = "일감화";
const STATUS_DISCARDED = "폐기";
const STATUS_ADOPTED = "채택";

const statusLabels = [ALL, STATUS_UNREVIEWED, STATUS_DONE, STATUS_DISCARDED, STATUS_ADOPTED];
const areaLabels = [ALL, "엔진", "에디터", "클라이언트", "그래픽", "물리", "사운드"];

function escapeHtml(value) {
  return String(value ?? "")
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;");
}

function statusClass(status) {
  if (status === STATUS_DONE) return "done";
  if (status === STATUS_DISCARDED) return "drop";
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
    if (area !== ALL && item.area !== area) return false;
    if (status !== ALL && item.status !== status) return false;
    if (!query) return true;
    return [item.title, item.situation, item.problem, item.suggestion]
      .join(" ")
      .toLowerCase()
      .includes(query);
  });
}

function renderStats() {
  const counts = {
    [ALL]: feedback.length,
    [STATUS_UNREVIEWED]: feedback.filter((item) => item.status === STATUS_UNREVIEWED).length,
    [STATUS_DONE]: feedback.filter((item) => item.status === STATUS_DONE).length,
    [STATUS_DISCARDED]: feedback.filter((item) => item.status === STATUS_DISCARDED).length,
    [STATUS_ADOPTED]: feedback.filter((item) => item.status === STATUS_ADOPTED).length,
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
  els.detail.innerHTML = `<div class="empty">데이터를 불러오지 못했습니다. ${escapeHtml(error.message)}</div>`;
});
