local root = "D:/PixelEngine/Survival/Asset/"
dofile(root .. "Table/MonsterList.lua")
dofile(root .. "Script/Manager/MonsterManager.lua")
dofile(root .. "Table/EnemyEffects.lua")
local pickup = dofile(root .. "Script/Entity/EXP.pxm")
pickup.ID = 100
local drops, received = 0,0
Pool = {}
function Pool.Active(name)
    assert(name == "EXP")
    drops = drops + 1
    return pickup.ID
end
function Pool.Disable(name,id)
    assert(name == "EXP" and id == pickup.ID)
end
Entity = {}
function Entity.CallFunction(id,name,value)
    assert(id == pickup.ID and name == "Setting")
    pickup:Setting(value)
end
Group = {}
function Group.Remove()


end
ffi = {}
function ffi.string(value)
    return value
end
local position
Rigidbody2D = {}
function Rigidbody2D.Get(id)
    local body = {}
    function body:SetPosition(value)
        position = value
    end
    function body:GetLayer()
        return id == 1 and "Player" or "Enemy"
    end
    return body
end
PlayerManager = {}
function PlayerManager.AddExp(value)
    received = received + value
end
for index, name in ipairs({"Bat","Skeleton","Bat"}) do
    local monster = dofile(root .. "Script/Entity/Monster/" .. name .. ".pxm")
    monster.ID = index
    monster.transform = {position = {x = index,y = 2,z = 0}}
    monster.MonsterState = {Move = 0,Hit = 1,Death = 2}
    monster.animationTabel = {move = 0,death = 1}
    monster.animation = {}
    function monster.animation:Play()


    end
    monster:ActiveSetting()
    local amount = name == "Skeleton" and 3 or 1
    assert(monster.EXP == amount)
    assert(monster:TakeDamage(monster.HP))
    assert(drops == index and pickup.ExpPoint == amount)
    assert(position == monster.transform.position)
    assert(not monster:TakeDamage(1) and drops == index)
    pickup:OnCollisionEnter(2)
    assert(not pickup.Collected)
    local before = received
    pickup:OnCollisionEnter(1)
    pickup:OnCollisionEnter(1)
    assert(received == before + amount,"Pickup duplicate or stale pooled EXP")
end
assert(received == 5)
print("PASS: Bat 1 / Skeleton 3, death position, duplicate death/contact and pooled pickup reset")
