local root = "D:/PixelEngine/Survival/Asset/"
local paused, nextID = false,100
local transforms, bodies, bullets, monsters, disabled = {}, {}, {}, {}, {}
local group, deaths = {},0
local function Near(actual,expected)
    assert(math.abs(actual - expected) < 0.000001,tostring(actual) .. " ~= " .. tostring(expected))
end
function PVector3(x,y,z)
    local vector = {x = x,y = y,z = z}
    function vector:Distance(other)
        return math.sqrt((self.x - other.x)^2 + (self.y - other.y)^2 + (self.z - other.z)^2)
    end
    function vector:Direction(other)
        local distance = self:Distance(other)
        return PVector3((other.x - self.x) / distance,(other.y - self.y) / distance,(other.z - self.z) / distance)
    end
    return vector
end
function PVector2(x,y)
    return {x = x,y = y}
end
Time = {}
function Time.IsPaused()
    return paused
end
ffi = {}
function ffi.string(value)
    return value
end
Transform = {}
function Transform.Add(id)
    transforms[id] = {position = PVector3(0,0,0),scale = PVector3(1,1,1)}
    return transforms[id]
end
function Transform.Get(id)
    return transforms[id]
end
Rigidbody2D = {}
function Rigidbody2D.Add(id)
    local body = {Layer = "Enemy"}
    function body:CreateCircleCollider()


    end
    function body:SetMotionType()


    end
    function body:SetLayer(layer)
        self.Layer = layer
    end
    function body:GetLayer()
        return self.Layer
    end
    function body:SetPosition(position)
        transforms[id].position = PVector3(position.x,position.y,position.z)
    end
    function body:SetVelocity(velocity)
        self.Velocity = velocity
    end
    bodies[id] = body
    return body
end
function Rigidbody2D.Get(id)
    return bodies[id]
end
Renderer2D = {}
function Renderer2D.Add()
    local renderer = {}
    function renderer:SetColor(r,g,b,a)
        self.Color = {r,g,b,a}
    end
    function renderer:SetTransparent(value)
        self.Transparent = value
    end
    return renderer
end
Animation2D = {}
function Animation2D.Add()
    local animation = {}
    function animation:CreateRegion()
        return 0
    end
    function animation:Play(index)
        self.Playing = index
    end
    function animation:SetLoop()


    end
    function animation:SetUseUnscaledTime()


    end
    function animation:Reset()


    end
    function animation:GetFrameIndex()
        return 0
    end
    return animation
end
MotionType = {Kinematic = 1}
Group = {}
function Group.GetCount()
    return #group
end
function Group.GetID(name,index)
    return group[index + 1]
end
function Group.Remove(name,id)
    deaths = deaths + 1
    for i, value in ipairs(group) do
        if value == id then table.remove(group,i) break end
    end
end
Pool = {}
function Pool.SetExpansionSize()


end
function Pool.Active(name)
    nextID = nextID + 1
    local id = nextID
    Transform.Add(id)
    Rigidbody2D.Add(id)
    if name == "Bullet" then
        local script = dofile(root .. "Script/Entity/Bullet.pxm")
        script.ID = id
        script:Spawn()
        bullets[id] = script
    end
    return id
end
function Pool.Disable(name,id)
    disabled[id] = true
end
Entity = {}
function Entity.CallFunction(id,name,...)
    if bullets[id] then return bullets[id][name](bullets[id],...) end
    if monsters[id] then return monsters[id][name](monsters[id],...) end
end
PlayerManager = {transform = {position = PVector3(0,0,0)}}
dofile(root .. "Table/AttributeList.lua")
dofile(root .. "Table/EnergyList.lua")
dofile(root .. "Table/EnemyEffects.lua")
dofile(root .. "Table/MonsterList.lua")
dofile(root .. "Script/Manager/MonsterManager.lua")
dofile(root .. "Script/Manager/BulletManager.lua")
BulletManager.Init()
local function Enemy(id,name)
    local enemy = dofile(root .. "Script/Entity/Monster/" .. (name or "Bat") .. ".pxm")
    enemy.ID = id
    enemy.transform = Transform.Add(id)
    enemy.transform.position = PVector3(id,0,0)
    enemy.animation = Animation2D.Add()
    enemy.animationTabel = {move = 0,hit = 1,death = 2}
    enemy.MonsterState = {Move = 0,Hit = 2,Death = 1}
    enemy.rigidbody = Rigidbody2D.Add(id)
    enemy:ActiveSetting()
    enemy.HP = 100
    monsters[id] = enemy
    return enemy
end

assert(AttributeList.Fire == nil and AttributeList.Ice == nil)
local skeleton = Enemy(11,"Skeleton")
skeleton:ActiveSetting()
Near(skeleton.HP,4)
Near(skeleton.Speed,1)
skeleton:Move()
Near(math.abs(skeleton.rigidbody.Velocity.x),1)
for _, removed in ipairs({"Fire","Ice"}) do
    local enemy = Enemy(12)
    enemy:TakeDamage(1,removed)
    EnemyEffects.Update(enemy,3)
    Near(enemy.HP,99)
    Near(EnemyEffects.GetSpeedMultiplier(enemy),1)
    assert(next(enemy.Effects) == nil)
end

local victim = Enemy(20)
victim.HP = 1
local before = deaths
victim:TakeDamage(1)
assert(victim.HP <= 0 and victim.State == victim.MonsterState.Death and deaths == before + 1)
EnemyEffects.Update(victim,3)
assert(deaths == before + 1 and EnemyEffects.Alive[20] == false)

group = {1,2,3,4,5}
for i = 1, 5 do Enemy(i) end
BulletManager.Create(8,1,"Electric",3)
local bulletID = nextID
local bullet = bullets[bulletID]
for i = 1, 4 do
    monsters[i]:OnCollisionEnter(bulletID)
    Near(monsters[i].HP,100 - 8 / 2^(i - 1))
    local remaining = BulletManager.Hits[bulletID].Remaining
    monsters[i]:OnCollisionEnter(bulletID)
    assert(BulletManager.Hits[bulletID].Remaining == remaining,"Duplicate collision consumed a bounce")
    if i < 4 then
        assert(not disabled[bulletID] and bullet.TargetID == i + 1)
        bullet:Update(0.1)
        assert(bullet.rigidbody.Velocity.x > 0,"Bounce projectile did not chase the next enemy")
    end
end
assert(disabled[bulletID] and BulletManager.Hits[bulletID].Consumed and monsters[5].HP == 100)
local activate = Pool.Active
function Pool.Active(name)
    if name == "Bullet" then disabled[bulletID] = nil return bulletID end
    return activate(name)
end
BulletManager.Create(10,1)
assert(BulletManager.Hits[bulletID].Element == nil and BulletManager.Hits[bulletID].Remaining == 0)
assert(next(BulletManager.Hits[bulletID].Visited) == nil and bullet.TargetID == nil)
assert(bullet.renderer.Color[1] == 255 and bullet.renderer.Color[2] == 255)
monsters[1]:OnCollisionEnter(bulletID)
assert(next(monsters[1].Effects) == nil)
group = {1}
BulletManager.Create(8,1,"Electric")
assert(BulletManager.Hits[bulletID].Remaining == 0,"No reactor must grant no extra bounce")
monsters[1]:OnCollisionEnter(bulletID)
assert(disabled[bulletID],"No next enemy must return the chain projectile to its pool")

for _, count in ipairs({1,2,3,4,5}) do
    BulletManager.Create(8,1,"Electric",count)
    assert(BulletManager.Hits[bulletID].Remaining == count)
end

group = {40,41,42,43,44}
for i = 40, 44 do Enemy(i) end
transforms[40].position = PVector3(1,0,0)
transforms[41].position = PVector3(1.5,0,0)
transforms[42].position = PVector3(2,0,0)
transforms[43].position = PVector3(2.01,0,0)
transforms[44].position = PVector3(3,0,0)
BulletManager.Create(8,1,nil,0,1)
monsters[40]:OnCollisionEnter(bulletID)
Near(monsters[40].HP,92)
Near(monsters[41].HP,98)
Near(monsters[42].HP,98)
Near(monsters[43].HP,100)
Near(monsters[44].HP,100)
monsters[40]:OnCollisionEnter(bulletID)
Near(monsters[41].HP,98)
assert(BulletManager.Hits[bulletID].Consumed)

BulletManager.Create(4,1,nil,0,2)
paused = true
monsters[40]:OnCollisionEnter(bulletID)
Near(monsters[40].HP,92)
paused = false
monsters[41].HP, monsters[42].HP = 1,1
monsters[40]:OnCollisionEnter(bulletID)
Near(monsters[40].HP,88)
assert(not EnemyEffects.Alive[41] and not EnemyEffects.Alive[42])
Near(monsters[43].HP,99)
Near(monsters[44].HP,99)
BulletManager.Create(8,1)
assert(BulletManager.Hits[bulletID].ExplosionRadius == 0)
monsters[40]:OnCollisionEnter(bulletID)
Near(monsters[43].HP,99)

-- Chain impacts use the current (already reduced) attack for each explosion.
group = {40,43,44}
BulletManager.Create(8,1,"Electric",1,2)
monsters[40]:OnCollisionEnter(bulletID)
Near(monsters[43].HP,97)
monsters[43]:OnCollisionEnter(bulletID)
Near(monsters[44].HP,96)
print("PASS: 25% explosion, radius boundaries, direct-hit exclusion, deaths during iteration, pool reset and chain combination")

-- A new data-defined attribute must work without branching on its name.
AttributeList.Poison = {Duration = 2,TickInterval = 0.5,TickCount = 4,TickDamageRatio = 0.2,Color = {0,255,0}}
local poison = Enemy(30)
poison:TakeDamage(10,"Poison")
EnemyEffects.Update(poison,2)
Near(poison.HP,82)
local r,g,b = EnergyList.GetColor("Poison")
assert(r == 0 and g == 255 and b == 0)
AttributeList.Mark = {OnHit = function(enemy,attack)
    enemy.Mark = attack
end}
poison:TakeDamage(1,"Mark")
assert(poison.Mark == 1)
print("PASS: skeleton HP/speed, removed fire/ice, death, moving chain 100/50/25/12.5, pooled reset and data-defined extension")
