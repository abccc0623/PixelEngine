local paused,damage,disabled = false,0,0
local position = {}
function position:Distance(target)
	return target.distance
end
function position:Direction(target)
	return {x=1,y=0,z=0}
end
PlayerManager = {transform={position=position}}
EnemyEffects = {Alive={[10]=true}}
Group = {GetCount=function()
	return 1
end,GetID=function()
	return 10
end}
Transform = {Get=function()
	return {position={distance=2}}
end}
Rigidbody2D = {Get=function()
	return {SetPosition=function()


	end}
end}
local effects = 0
Pool = {SetExpansionSize=function()


end,Active=function(name)
	if name == "HitEffect" then
		effects = effects + 1
		return 2
	end
	assert(name == "Bullet")
	return 1
end,Disable=function(name,id)
	assert(name == "Bullet" and id == 1)
	disabled = disabled + 1
end}
Entity = {CallFunction=function(id,name,...)
	if name == "ActiveSetting" and id == 1 then
		assert(select("#",...) == 2)
	end
end}
Time = {IsPaused=function()
	return paused
end}
dofile("Asset/Script/Manager/BulletManager.lua")
BulletManager.Init()
assert(BulletManager.Explode == nil and BulletManager.FindChainTarget == nil)
local enemy = {HP=5,transform={position=position}}
function enemy:TakeDamage(value,...)
	assert(select("#",...) == 0)
	damage = damage + value
	self.HP = self.HP - value
	return true
end
BulletManager.Create(1,1)
BulletManager.Hit(1,enemy)
BulletManager.Hit(1,enemy)
assert(damage == 1 and disabled == 1 and effects == 1)
BulletManager.Create(1,1)
paused = true
BulletManager.Hit(1,enemy)
assert(damage == 1)
paused = false
BulletManager.Hit(1,enemy)
assert(damage == 2 and disabled == 2)
local bullet = dofile("Asset/Script/Entity/Bullet.pxm")
assert(bullet.Bounce == nil)
print("basic_bullet: PASS")
