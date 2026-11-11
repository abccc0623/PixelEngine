# Engine capability catalog

This is the canonical, engine/editor-owned capability knowledge shipped inside PixelTool, not a game-project inventory. Status describes the inspected source implementation, not a verified installed DLL or completed gameplay. Source references below are relative to PixelSolution; symbols are stable lookup anchors, not instructions to access engine sources from a game-only session. Verify the selected project's generated Engine/*.lua declarations before using an API. If declarations, source evidence or observed behavior disagree, report unknown for that route and inspect the smallest relevant evidence. Absence from this catalog means unknown, never automatically unsupported.

## Reading and status rules

Read this section and only the relevant complete capability sections with read_engine_guide. search_engine_guide only locates them. Do not load every API or inspect all projects. API names/types/signatures remain owned by BindManager metadata and its generated Engine/*.lua files; this catalog records semantic capabilities and exceptions, not a second complete API definition.

- supported: the specified engine primitive has source-backed implementation. This does not mean the requested game feature exists or passed runtime tests.
- partial: some primitives exist, but named game Lua logic or integration is still required, or a documented limit prevents promising the whole feature.
- not-supported (unavailable in scope reports): a specific route is confirmed absent or a stub in the inspected source. Do not generalize it to all possible Lua algorithms.
- unknown: missing, ambiguous, conflicting or insufficient evidence. Identify the next small check, not a fabricated workaround or an unconditional rejection.

Per feature, report: requested behavior; catalog section/status; exact relevant API and project declaration reference; existing project logic versus new Lua work; limits/assets; completion checks and what is untested. Supported input is not a character controller. A reference game is design evidence for observable mechanics, not proof of engine support. Do not invent reference research, exact copyrighted source/assets or game-specific tuning values.

## Input and time

Status: supported for keyboard/mouse-button polling and time; game-specific bindings/controllers are partial until implemented.
- Lua: Input.Key(key), Input.KeyDown(key), Input.KeyUp(key) return booleans and use Keycode; Input.GetMousePositionX()/GetMousePositionY() return coordinates.
- Evidence: PixelEngine/Input.cpp, Input_Key/Input_KeyDown/Input_KeyUp and ToVirtualKey; PixelEngine/KeyInputManager.cpp; declarations Input.h and metadata BindManager.cpp. Keycode.Left/Right/Up/Down/Space are mapped.
- Lua: Time.GetDeltaTime(), Time.GetUnscaledDeltaTime(), Time.GetTotalTime(), Time.GetFPS(), Time.SetPaused(paused), Time.IsPaused(). Evidence: PixelEngine/TimeAPI.cpp and TimeManager.cpp.
- Limits: these do not define movement, jump counts, grounded state, remapping, controller/gamepad or touch support. Those additional input devices are unknown here. Mouse coordinates are not automatically world coordinates. Use pressed-this-frame versus held deliberately.

## Entities, transforms and Lua lifecycle

Status: supported for entity/component primitives and Lua-driven behavior.
- Lua: Entity.Create(scriptName), Entity.CreateEmpty(), Entity.Destroy(ID), Entity.SetActive(ID,active), Entity.GetActive(ID), Entity.GetScript(ID). Components use Type.Add(ID), Type.Get(ID), Type.Has(ID); check failed/nil results.
- Transform component data exposes position, rotation and scale; transform:SetParent(parentID), transform:SetChild(childID) provide hierarchy. Evidence: PixelEngine/Entity.cpp, Transform.cpp, TransformSystem.cpp and BindManager.cpp.
- Lua modules run Spawn/Update(DTime); scenes have Start/Update/Release. Entity.GetScript is supplied by PixelEngine/Setting.lua, not just the FFI metadata. Scene.Change(sceneName) is implemented through SceneManager.cpp/LuaSceneInfo.cpp. WaitForSeconds(seconds) is registered in LuaManager.cpp for the engine coroutine context; follow LuaApiGuide lifecycle restrictions.
- Limits: Transform_SetLocalPosition/Transform_SetLocalRotation/Transform_SetLocalScale in Transform.cpp are empty and not registered in current BindManager. They are not a supported Lua route. Do not confuse these stubs with writable transform data. Transform edits are not a substitute for Rigidbody2D physics movement. Controllers, combat rules and state machines remain game Lua.

## Sprites and animation

Status: supported for 2D sprite rendering and sprite-sheet animation; direct Renderer2D UV-field behavior is unknown.
- Lua component methods: renderer:SetTexture(name), renderer:SetColor(r,g,b,a), renderer:SetTransparent(boolean). Evidence: PixelEngine/Renderer2D.cpp -> GraphicsSystem.cpp -> PixelGraphics/ScenePass.cpp.
- Animation2D: animation:Create(textureName,maxFramesX,maxFramesY,speed), animation:CreateRegion(textureName,startX,startY,frameWidth,frameHeight,frameCount,speed), animation:Play(index), animation:Stop(), animation:Reset(), animation:GetFrameIndex(index), animation:SetFrameIndex(index,frameIndex), animation:SetLoop(index,loop), animation:SetUseUnscaledTime(boolean), animation:SetFrameCallback(index,frameIndex,repeat).
- Evidence: PixelEngine/Animation2D.cpp and Animation2DSystem.cpp; callback spelling Script:OnAnimationCallBack(targetID,nowAnimationID,nowAnimationFrame) in EntityObject.cpp. Indices are zero-based; requires Renderer2D or UIImage to be visible.
- Limits: imported artwork, clips, facing/idle/run/jump transitions and animation timing are game work. Renderer2DData TilingX/TilingY/OffsetX/OffsetY are registered but their transfer into graphics is not established by current GraphicsSystem.cpp; do not promise those direct fields work. Animation2D's own UV application is a separate implemented route.

## Camera and 3D boundary

Status: supported for camera primitives; partial for a general 3D game pipeline.
- Lua: Camera.Add/Get/Has; Camera data Projection, FovY, NearZ, FarZ, ZoomLevel and ViewportX/Y/Width/Height with Transform. Evidence: PixelEngine/Camera.h, BindManager.cpp, CameraSystem.cpp -> PixelGraphics/CameraManager.cpp Setting/ViewPortSetting.
- Limits: 2D follow, bounds, shake and camera transitions require game logic. Vectors, perspective projection and transforms do not establish a complete 3D engine.
- Current generic model import route is not-supported: PixelGraphics/ModelFactory.cpp ModelFactory::Load searches existing built-in models then returns the default quad. Skeletal animation/lighting beyond this evidence remain unknown; do not promise them from camera or vector APIs.

## UI primitives

Status: supported for images, text and fill bars; partial for complete interactive UI.
- Lua components: UIImage (SetTexture/SetSize/SetColor/SetPivot/SetOrder), UIText (SetText/SetTextSize/SetFont/SetWrapWidth/SetColor), UISilder (SetRange/SetValue/SetSize). Preserve the actual UISilder spelling.
- Evidence: PixelEngine/UIImage.cpp, UIText.cpp, UISilder.cpp and their *System.cpp files; method signatures from BindManager.cpp and project declarations.
- Limits: UISilder draws a fill bar, not a guaranteed draggable slider. Button hit-testing, menus, inventory, navigation and responsive layout are not supplied by these names; inspect/propose game logic separately. Font/images and user interactions need their own completion checks.

## Physics and collision

Status: supported for the listed 2D primitives; partial for a platformer controller.
- Lua: Rigidbody2D.Add/Get/Has; body:CreateBoxCollider(size,center), body:CreateCircleCollider(radius,center), body:SetMotionType(type), body:SetPosition(position), body:SetRotation(rotation), body:SetVelocity(velocity), body:SetGravity(gravity), body:SetLinearDamping(value), body:SetSensor(boolean), body:SetActive(boolean), body:SetLayer(name), body:GetLayer().
- body:SetLockPosition(x,y,z), body:SetLockRotation(x,y,z) return success; true locks the corresponding world axis. Verify vectors/enums in Engine/Rigidbody2D.lua. Evidence: PixelEngine/Rigidbody2D.cpp, PhysicsFunction.cpp, Rigidbody2DSystem.cpp and PhysManager.cpp.
- Lua callbacks: Script:OnCollisionEnter(targetID), Script:OnCollisionExit(targetID). Evidence: PhysListener.cpp -> PhysManager.cpp -> EntityObject.cpp. They pass the other ID, not a contact-normal/grounded result; internal contact data is not automatically exposed to Lua.
- Limits: XY-centered physics paths force Z to zero. Rigidbody2D_SetVelocity replaces a full vector; there is no registered Rigidbody2D_GetVelocity in current BindManager.cpp. Do not invent a getter or overwrite vertical velocity blindly during horizontal movement. Confirm a vertical-state strategy against the selected project before promising a controller.
- Setup/order limits: Physics2DData.h defaults to Kinematic and sensor=true. A physics-driven gravity/solid-floor route needs explicit Dynamic motion and SetSensor(false). SetGravity is a gravity factor, not an acceleration vector (PhysicsFunction::SetGravity). SetVelocity before body creation is not buffered (Rigidbody2D.cpp/Rigidbody2DSystem::Update); verify lifecycle order. CreateBoxCollider/CreateCircleCollider prepare the shape before body creation, not a proven live body-shape replacement API. Do not promise runtime crouch/resize through them.
- Known not-supported setters: body:SetRestitution(value), body:SetFriction(value) are registered/exported but Rigidbody2D.cpp bodies are empty. Declaration presence is not implementation. Do not plan friction or bounce tuning through them.
- Ground detection, landing reset, one-way/drop-through platforms, collision filtering per pair, slopes and moving-platform behavior need specific evidence/design. A sensor toggle affects the whole body; it is not a proven per-platform drop-through solution.

## Pool, group, events and tween

Status: supported as game-logic building blocks.
- Lua: Pool.Active(scriptName), Pool.Disable(scriptName,ID), Pool.SetExpansionSize(scriptName,count), Pool.GetActiveCount(scriptName), Pool.GetActiveID(scriptName,index); Group.Set(name,ID), Group.Remove(name,ID), Group.GetCount(name), Group.GetID(name,index).
- Lua: Event.Bind(ID,eventName,functionName), Event.Call(eventName); Tween.Move(ID,property,startValue,endValue,duration,useUnscaledTime,ease), Tween.PingPong(ID,property,minValue,maxValue,duration,useUnscaledTime,ease), Tween.Pause/Resume/Kill(ID,property).
- Evidence: PixelEngine/PoolManager.cpp, GroupManager.cpp, EventManager.cpp, TweenManager.cpp and BindManager.cpp; consult LuaApiGuide for lifecycle/iteration constraints.
- Limits: pool/group state is scene-local; reuse needs explicit game-state reset. Tweens cover transform position/rotation/scale, not an arbitrary-property animation system. These APIs do not implement enemies, attacks, cooldown rules or quests.

## Assets, data and diagnostics

Status: supported for listed imports/diagnostics; partial for persistence.
- Lua: Asset.Import(path); Debug.Log(text), Debug.LogWarning(text), Debug.LogError(text), Debug.Line(startPoint,endPoint,color). Evidence: PixelEngine/Asset.cpp ImportFile, Debug.cpp and BindManager.cpp.
- ImportFile explicitly handles .lua/.scene/.pxm, .png/.jpg, .mat, .ttf/.otf and .json. JsonManager.cpp implements LayerMatrix configuration loading. File-copy support in editor tools is not proof of runtime import.
- Known not-supported route: JsonManager::LoadScene/LoadGameObject have empty bodies. JsonManager::Save being a C++ helper is not a public Lua save-game API. General save/load, arbitrary JSON tables and cross-scene persistence are unknown until a specific route is verified.
- Limits: logs/reimport do not prove gameplay or rendering correctness. User assets, format compatibility and import order are separate from primitive support.

## Sound and game networking

Status: not-supported for built-in game audio playback and multiplayer/network transport in the inspected public engine pipeline.
- Evidence boundary: PixelEngine/BindManager.cpp public registration, LuaManager.cpp extra Lua registration, Asset.cpp ImportFile and engine/graphics/client first-party source inventory contain no game playback/transport implementation. No engine Lua audio/network API is provided by this catalog.
- Asset.cpp does not import .wav/.ogg/.mp3. Editor asset copy/inspection accepting those extensions does not create runtime sound support. PixelTool AI HTTP/MCP communication is editor infrastructure, not game networking.
- Do not fabricate Audio.Play, Sound.Play, Network.Connect or similar APIs. Defer sound/multiplayer in a first playable scope. An external library or a later engine addition requires separate evidence and authorization, not an assumption that it is installed. Other unlisted capabilities stay unknown.

## Reference movement planning example

Use this as a conditional reasoning example, not a fixed genre template or an implemented feature list. For "메이플 같은 이동", first read Input and time, Physics and collision, and Entities, transforms and Lua lifecycle, then relevant project declarations/callers. Analyze only the requested observable mechanics; if no reference evidence was supplied or inspected, label interpretations as proposed and ask only when the distinction changes the plan.

- 좌우 이동 (left/right): input primitive supported; controller partial. Plan key sampling, speed/acceleration/deceleration and a verified horizontal/vertical state strategy. Do not treat Input.Key as completed movement.
- 윗점프 (upward jump): partial/proposed Lua behavior using pressed input plus vertical-state and collision strategy. Establish grounded/takeoff/landing rules before choosing API calls. "Up" may mean a special upward skill rather than the ordinary jump; preserve the user's meaning.
- 더블점프 (double jump): partial/proposed Lua state logic, not a built-in API. Plan airborne jump count, trigger, direction/velocity, allowed transitions and reset-on-verified-landing. Input and SetVelocity alone do not prove these rules.
- 아래점프 (downward/drop-through jump): unknown until the intended behavior and selective platform interaction are established. Distinguish downward impulse from dropping through one-way floors. Do not silently substitute global SetSensor or claim one-way support.
- After capability judgment, create dependency-ordered game Lua tasks for input mapping -> motion/grounding -> requested jump variants -> animation/camera integration -> completion checks. Choose exact project paths only after focused inspection; do not create or alter a game from this example alone.
- Completion checks, when authorized/available: left/right response and release; no unintended vertical-speed reset; jump only in allowed states; no third jump; reset after a valid landing rather than wall contact; drop-through affects only intended floors; no new errors. Separate planned checks from actually run checks.

For a broad platformer request, propose a small playable movement/landing slice based on established primitives, explicitly defer unknown drop-through behavior and unavailable sound/network. For an ambiguous request such as "그 게임처럼 이동을 특별하게", report unknown requirements, state the few plausible interpretations and ask the smallest material question. A name, catalog match or zero search results does not establish feasibility.

## Maintenance

The single source for these semantic statuses is PixelTool/AI/EngineCapabilities.md, embedded by PixelTool.csproj and served by EngineGuide to the manager, programmer and MCP. Never copy it into each game's Asset/Editor folder or maintain a second project-specific support list.

API metadata remains in PixelEngine/BindManager.cpp (plus explicit Lua registrations/lifecycle code). Existing generation is EditorManager::Notify(GenerateEngineFile) -> GenerateManager::JsonGenerate -> LuaStaticCreate/LuaComponentCreate/LuaTypeCreate/LuaEnumCreate. It produces Engine/*.lua declarations; do not infer implementation status from automatic generation. GenerateManager::LuaGenerate/LSPGenerate are inactive stubs, not an extension point.

When a primitive, binding or implementation changes, update the relevant semantic entry/evidence and regression checks in the same change, then rebuild PixelTool. Never promote an unlisted API to supported automatically. No engine-version/DLL compatibility system is introduced here. Current project declarations and real behavior still need focused verification; missing/stale evidence becomes unknown.
