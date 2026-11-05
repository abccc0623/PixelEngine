# PixelEngine Lua API guide

Embedded PixelTool reference, not model training, execution authority or a file to copy into game Asset/engine DLLs. AIRestrictions.md governs deployment permissions.

## Verify before use

Use the opened project's `Engine/*.lua` for exact names, types, parameters and returns; consult AGENTS.md/project guides as supplements. Report version mismatches. Never invent APIs or borrow names from another engine. Do not assume C++ sources exist on a deployed PC or inspect/modify DLL internals; offer supported Lua alternatives.

## Entry, scenes and lifetime

- Engine reads `Asset/main.lua`, then calls global `Main()`.
- Every Entity belongs to a scene. Creating objects without an explicit current scene prepares the internal default scene. After `Scene.Change("sceneName")`, new objects belong to that scene; existing objects do not automatically migrate.
- Creating/importing/registering a scene file does not run it: select it in main with Scene.Change.
- Scene template: `local Scene = {}`, `Scene:Start()`, `Scene:Release()`, `return Scene`. Start runs on scene change. Lua Release invocation is currently disabled; do not promise cleanup callbacks merely because the template contains one.
- ECS object creation defaults to `Entity.Create("scriptName")`: one Entity ID paired with one `.pxm` script, not merged Lua files. Verify import coverage first; honor explicit scriptless requests.
- Create finishes object construction and `Spawn()` before returning its ID. Spawn runs once, not on pool reuse. Keep object behavior/components/default `self` values in Spawn and the object's own script. Setting/Initialize are optional user-chosen names, not required engine callbacks.
- `Entity.CreateEmpty()` returns a scriptless ID.
- `Entity.GetScript(ID)` returns the registered Lua table, or nil if absent. Access fields directly and call methods with the definition's dot/colon convention: `local target = Entity.GetScript(targetID)`, then `target:Attack()`. Avoid retaining unnecessary long-lived script-table references.
- Embedded Setting.lua manages LuaManager.entities registration/removal. Game-specific references/lists/state are the client's responsibility.
- `Entity.Destroy(ID)` deletes. Audit callers holding its ID/table/component pointers for stale use.
- `Entity.GetActive(ID)` queries activation; do not invent Entity.IsActive. `Entity.SetActive(ID,false)` disables, not deletes: Lua Update and ECS-managed collision/animation stop.
- IDs, Lua tables and component data are different. Do not compare IDs to arbitrary numeric sentinels or add speculative validation.

## Cameras and creation helpers

`Entity.CreateCamera()` creates Transform + Camera and returns **ID**.
`Entity.Create2DImage(textureName)` creates Transform + UIImage, sets texture and returns **UIImageData**, not ID. Both are native Entity APIs.

If only background color changes and nothing renders, first check that the active scene actually creates a camera. Then check activation, Transform/position, projection and object visibility bounds. Background color alone proves neither camera nor object rendering.

```lua
local cameraID = Entity.CreateCamera()
local transform = Transform.Get(cameraID)
transform.position.z = -20
local camera = Camera.Get(cameraID)
camera.ZoomLevel = 50
```

A camera does not require its own .pxm. Do not mistake helper return types for component data/IDs.

## Loading, data and code style

- `Asset.Import(path)` loads a file or recursively imports a folder. Import calls finish in order: import global tables first, scripts next, then create objects; Spawn can then use the tables. Do not assume a folder's internal traversal resolves dependencies.
- Avoid require/dofile/loadfile or duplicate/custom loaders in game scripts. Use engine loading; external-library exceptions need a verified reason. Do not remove engine-internal require("ffi") or other required engine dependencies.
- Match the editor's current object template: `local Script = {}` → `Script:Spawn()` with `self.transform = Transform.Add(self.ID)` → `Script:Update(DTime)` → `return Script`. Direct file creation is allowed. Preserve template order/initialization/comments, not invented boilerplate.
- Initialize per-object values such as self.moveSpeed/self.spawnInterval in Spawn, not top-level constants or the creating scene/main.
- Functions/variables and file/folder names are English. Use tabs for nesting and aligned multiline function/if/loop/end blocks; never compress even trivial functions to one line. Calls may remain one line. Do not reformat unrelated code.
- Add a short Korean `---` role description immediately above each new/modified function, including Spawn/Update. Preserve accurate comments; do not add filler or inaccurate descriptions.
- Global designer-editable lists belong in `Asset/Table/`, separate from `Asset/Script/` logic. Prefer data only; a trivial single-body-line lookup helper is allowed but still uses multiline definition/comment. Object-local tables remain in the object's script.
- Handle keys normally in the relevant object's script; do not impose a central input framework.
- Colon passes self. Normal Lua arrays are 1-based; API indices below may be 0-based.

## Components, Transform and UI

Components: `Transform`, `Camera`, `Renderer2D`, `Rigidbody2D`, `Animation2D`, `UIImage`, `UIText`, `UISilder` (actual spelling, not UISlider).
Each exposes `Add(ID)`, `Get(ID)`, `Has(ID)`; Add/Get return component data, not ID. Use component methods with colon.
- Ordinary objects use Renderer2D; UI uses UIImage/UIText. Do not invent Renderer.Add or swap world/UI components.
- Transform fields: `position`, `rotation`, `scale`; vector fields: lowercase x/y/z. `SetParent(parentID)`/`SetChild(childID)` link IDs. Child position is parent-local; parent position/scale affect children. Verify reparent preservation separately. Do not attach duplicate children or create cycles.
- UI pivots are normalized 0–1; center = (0.5,0.5). Distinguish local/world/screen coordinates.
- Camera fields: Projection, FovY, NearZ, FarZ, ZoomLevel, ViewportX, ViewportY, ViewportWidth, ViewportHeight. Use the project's ProjectionType enum.

## Image and text methods

- Renderer2D: `SetTexture(textureName)`, `SetColor(R,G,B,A)`, `SetTransparent(boolean)`; UV fields TilingX/TilingY/OffsetX/OffsetY.
- **Renderer2D, UIImage and UIText RGBA use 0–255, not 0–1.** Opaque white: SetColor(255,255,255,255); red: SetColor(255,0,0,255). Renderer2D clamps and normalizes internally.
- UIImage: `SetTexture(name)`, `SetTextureSize(width,height)`, `SetAspectFit(slotWidth,slotHeight)`, `SetContentAspectFit(slotWidth,slotHeight)`, `SetPivot(x,y)`, `SetTiling(x,y)`, `SetOffset(x,y)`, `SetOrder(order)`, `SetColor(r,g,b,a)`.
- AspectFit preserves the whole image's aspect ratio within the slot. ContentAspectFit excludes transparent margins and changes UVs; check interaction with animation UV updates. Both return success.
- UIText: `SetText(text)`, `SetFont(fontName)`, `SetTextSize(scale)`, `SetWrapWidth(width)`, `SetPivot(x,y)`, `SetColor(r,g,b,a)`. Text size is a scale; wrap width ≤0 disables wrapping. Wrapping is not line-height control; do not invent a line-height API.
- UISilder: `SetRange(min,max)`, `SetValue(value)`, `SetSize(width,height)`, `SetBackgroundColor(r,g,b,a)`, `SetFillColor(r,g,b,a)`, `SetBackgroundTexture(name)`, `SetFillTexture(name)`, `SetOrder(order)`.

## Time and tweening

- `Time.SetPaused(true/false)` pauses/resumes; `Time.IsPaused()` queries.
- While paused, Time.GetDeltaTime() and Update(DTime)'s DTime are 0; active objects in play mode still receive Update. Non-time-based logic does not automatically stop.
- `Time.GetUnscaledDeltaTime()` progresses during pause (UI/build previews). `Time.GetTotalTime()` also ignores game pause; it is not paused gameplay time. Time pause differs from Entity deactivation.
- `Tween.Move(ID,Property,StartValue,EndValue,Duration,UseUnscaledTime,Ease)`: one transition.
- `Tween.PingPong(ID,Property,MinValue,MaxValue,Duration,UseUnscaledTime,Ease)`: repeats; Duration is one-way time.
- Property uses TweenProperty Position/Rotation/Scale X/Y/Z. UseUnscaledTime=true runs through game pause. Optional last Ease defaults to TweenEase.Linear; In/Out/InOut also exist.
- New tween on the same ID/property replaces the old one; other properties coexist. `Tween.Pause/Resume/Kill(ID,Property)`; Kill preserves the current value.
- Disabled objects pause progress; reactivation resumes. Deletion removes their tweens.

## Animation2D

Requires a visible Renderer2D or UIImage, not Animation2D alone.
- `Create(textureName,maxFrameX,maxFrameY,speed)` divides the sheet by frame counts and returns animation index; do not assume speed means total duration in seconds.
- `CreateRegion(textureName,startX,startY,frameWidth,frameHeight,frameCount,speed)` uses horizontally consecutive frames in a sheet; positions/sizes are pixels.
- `Play(animationIndex)`, `Stop()`, `Reset()`, `GetFrameIndex(animationIndex)`, `SetFrameIndex(animationIndex,frameIndex)`; animation and frame indices are **0-based**.
- `SetLoop(animationIndex,loop)`, `SetUseUnscaledTime(boolean)`.
- `SetFrameCallback(animationIndex,frameIndex,repeat)` invokes `Script:OnAnimationCallBack(targetID,nowAnimationID,nowAnimationFrame)`; preserve spelling/case.

## Input and events

- `Input.Key(Keycode.W)`: held; `KeyDown`: pressed this frame; `KeyUp`: released. Use Keycode, not "W" or Windows virtual-key numbers.
- `Input.GetMousePositionX()/GetMousePositionY()`: do not assume returned coordinates are world or parent-local UI positions.
- `Event.Bind(ID,eventName,bindFunctionName)` registers a function-name string; `Event.Call(eventName)` triggers. Both return success.

## Pool and Group

Both belong to the current scene, not a cross-scene global store.
- `Pool.Active(scriptName)` returns an activated **ID**; creates a batch when exhausted. `Pool.SetExpansionSize(scriptName,createCount)`: positive batch size.
- `Pool.Disable(scriptName,ID)` returns/disables an instance, not destroys it. Client resets position/HP/etc. on reuse; Spawn does not rerun.
- `Pool.GetActiveCount(scriptName)`, `Pool.GetActiveID(scriptName,index)`: index **0..count-1**. Returning objects during iteration mutates the list; avoid skipping entries.
- Pool.Clear(scriptName) only clears active/inactive lists: it does **not** Destroy or automatically disable objects.
- `Group.CreateGroupAndEntity(groupName,scriptName)` creates/registers and returns ID. `Group.Set(groupName,ID)` registers; `Remove(groupName,ID)` unregisters; `Clear(groupName)` clears membership, not objects.
- `Group.GetCount(groupName)`, `Group.GetID(groupName,index)`: **0..count-1**. Legacy Count/First exist; First has no guaranteed empty-group return, so do not call on an empty group.

## Physics and collisions

Rigidbody2D methods (verify vector types/enums in Engine declarations):
- `CreateBoxCollider(size,center)`, `CreateCircleCollider(radius,center)`, `SetMotionType(MotionType)`.
- `SetPosition(position)`, `SetRotation(rotation)`, `SetVelocity(velocity)`: do not assume changing Transform updates physics.
- `SetLockPosition(x,y,z)` locks world translation axes; `SetLockRotation(x,y,z)` locks rotation axes. Boolean **true locks, false unlocks**; return success. Translation/rotation settings preserve each other; callable in Spawn before body creation or later.
- 2D: SetLockPosition(false,false,true), SetLockRotation(true,true,false). Prevent character tipping: SetLockRotation(true,true,true). Default is unrestricted; set locks explicitly.
- All six axes locked internally uses a static body; unlocking restores the configured motion type. These are simulation freedoms, not angle ranges, and do not prohibit explicit SetPosition/SetRotation.
- If lock APIs are missing, check engine version/Engine Update. Lua declarations alone cannot add exports to an old DLL.
- Other setters: SetGravity, SetRestitution, SetFriction, SetLinearDamping, SetSensor, SetActive, SetLayer(layerName); GetLayer(). Rigidbody activation is not whole-Entity activation.
- Callbacks: `Script:OnCollisionEnter(targetID)`, `Script:OnCollisionExit(targetID)` receive the other ID.

## Vectors, logs and engine settings

- `PVector2(x,y)`, `PVector3(x,y,z)`; PVector3 methods Normalize, Length, Distance, Dot, Cross, Direction, Lerp, ApproximatelyEquals. Do not operate on nil/plain tables as vectors.
- `Debug.Log(text)`, `Debug.LogWarning(text)`, `Debug.LogError(text)`; use tostring where needed, not invented Debug.LogInfo.
- `Debug.Line(startPoint,endPoint,color)`: verify vector argument declarations.
- `Engine.BackgroundColor(r,g,b)`, `Engine.SetWindowSize(width,height)`; window size is not editor docking-pane size.

API declarations, successful reimport and verified runtime behavior are not equivalent. If generated declarations differ, inspect the project's current version and report the mismatch instead of guessing.
