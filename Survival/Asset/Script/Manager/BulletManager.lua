BulletManager = {}
function BulletManager.Init()
    BulletManager.Attacks = {}
    BulletManager.Hits = {}
    Pool.SetExpansionSize("Bullet",1)
end

function BulletManager.Create(attack,itemIndex)
    local targetID = BulletManager.GetPoolMinDistanceTargetID("Bat")
    if targetID ~= -1 then
        local OrcPos = Transform.Get(targetID).position
        local playerPos = PlayerManager.transform.position
        local Direction = playerPos:Direction(OrcPos)

        local id = Pool.Active("Bullet")
        BulletManager.Attacks[id] = attack or 1
		BulletManager.Hits[id] = false
        local rigidbody = Rigidbody2D.Get(id)
        rigidbody:SetPosition(playerPos)
        Entity.CallFunction(id,"ActiveSetting",Direction,itemIndex or 1)
    end
end


function BulletManager.GetAttack(id)
    return BulletManager.Attacks[id] or 0
end

function BulletManager.Hit(id,enemy)
	if Time.IsPaused() or enemy.HP <= 0 or BulletManager.Hits[id] then
		return
	end
	BulletManager.Hits[id] = true
	if enemy:TakeDamage(BulletManager.GetAttack(id)) then
		local effectID = Pool.Active("HitEffect")
		Entity.CallFunction(effectID,"ActiveSetting",enemy.transform.position)
	end
	Pool.Disable("Bullet",id)
end

function BulletManager.GetPoolMinDistanceTargetID(PoolKeyName)
    local size = Group.GetCount("MonsterList")
    local minDistance = 10
    local minTargetID = -1
    if size ~= 0 then
        for i = 0, size -1 do
            local targetID = Group.GetID("MonsterList", i)
            local targetTransform = Transform.Get(targetID)
            local playerPos = PlayerManager.transform.position
            local targetDistance = playerPos:Distance(targetTransform.position)

            if minDistance > targetDistance then
                minDistance = targetDistance
                minTargetID = targetID
            end
        end
    end
    return minTargetID
end
