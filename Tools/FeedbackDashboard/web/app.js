let items = [];
let selectedId = null;
let filter = "all";

const statusOrder = ["미검토", "일감화", "폐기", "채택"];

const $ = (id) => document.getElementById(id);

function countStatus(status) {
  return items.filter((item) => item.status === status).length;
}

function renderStats() {
  $("stats").innerHTML = ["미검토", "일감화", "폐기", "전체"]
    .map((name) => {
      const count = name === "전체" ? items.length : countStatus(name);
      return `<div class="stat"><span>${name}</span><strong>${count}</strong></div>`;
    })
    .join("");
}

function chipClass(status) {
  if (status === "일감화") return "done";
  if (status === "폐기") return "drop";
  return "";
}

function renderList() {
  const visible = items.filter((item) => filter === "all" || item.status === filter);
  $("feedbackList").innerHTML = visible.map((item) => `
    <button class="item ${item.id === selectedId ? "active" : ""}" data-id="${item.id}">
      <span class="item-title">${item.title}</span>
      <span class="chips">
        <span class="chip area">${item.area}</span>
        <span class="chip ${chipClass(item.status)}">${item.status}</span>
        <span class="chip">${item.date}</span>
      </span>
    </button>
  `).join("");

  document.querySelectorAll(".item").forEach((button) => {
    button.addEventListener("click", () => {
      selectedId = button.dataset.id;
      render();
    });
  });
}

function selectedItem() {
  return items.find((item) => item.id === selectedId) || items[0];
}

function renderDetail() {
  const item = selectedItem();
  if (!item) {
    $("detailPanel").innerHTML = `<div class="empty">피드백이 아직 없어.</div>`;
    return;
  }
  selectedId = item.id;
  $("detailPanel").innerHTML = `
    <article class="detail">
      <h2>${item.title}</h2>
      <div class="meta">
        <span class="chip area">${item.area}</span>
        <span class="chip ${chipClass(item.status)}">${item.status}</span>
        <span class="chip">${item.date}</span>
      </div>

      <div class="section">
        <h3>상황</h3>
        <p>${escapeHtml(item.situation)}</p>
      </div>
      <div class="section">
        <h3>문제</h3>
        <p>${escapeHtml(item.problem)}</p>
      </div>
      <div class="section">
        <h3>제안</h3>
        <p>${escapeHtml(item.suggestion)}</p>
      </div>

      <div class="section">
        <h3>폐기 사유</h3>
        <textarea id="discardReason" placeholder="폐기할 때만 적어도 돼.">${escapeHtml(item.discardReason || "")}</textarea>
      </div>

      <div class="actions">
        <button class="action accept" data-decision="accept">채택하고 일감화</button>
        <button class="action discard" data-decision="discard">폐기</button>
        <button class="action reset" data-decision="reset">미검토로 되돌리기</button>
      </div>
    </article>
  `;

  document.querySelectorAll(".action").forEach((button) => {
    button.addEventListener("click", () => decide(button.dataset.decision));
  });
}

function render() {
  items.sort((a, b) => {
    const dateCompare = b.date.localeCompare(a.date);
    if (dateCompare) return dateCompare;
    return statusOrder.indexOf(a.status) - statusOrder.indexOf(b.status);
  });
  renderStats();
  renderList();
  renderDetail();
}

function escapeHtml(value) {
  return String(value)
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;");
}

async function load() {
  const response = await fetch("/api/feedback");
  const data = await response.json();
  items = data.items;
  if (!selectedId && items.length) selectedId = items[0].id;
  render();
}

async function decide(decision) {
  const reason = $("discardReason")?.value || "";
  const response = await fetch(`/api/feedback/${encodeURIComponent(selectedId)}/decision`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ decision, reason }),
  });
  if (!response.ok) {
    const data = await response.json();
    alert(data.error || "처리 실패");
    return;
  }
  await load();
}

document.querySelectorAll(".filter").forEach((button) => {
  button.addEventListener("click", () => {
    filter = button.dataset.filter;
    document.querySelectorAll(".filter").forEach((item) => item.classList.remove("active"));
    button.classList.add("active");
    render();
  });
});

$("today").textContent = new Date().toLocaleDateString("ko-KR", {
  year: "numeric",
  month: "long",
  day: "numeric",
  weekday: "long",
});

load();
