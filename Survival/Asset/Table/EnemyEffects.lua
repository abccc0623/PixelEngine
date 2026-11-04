EnemyEffects = {Alive = {}}

function EnemyEffects.Reset(enemy)
    EnemyEffects.Alive[enemy.ID] = true
end

function EnemyEffects.TakeDamage(enemy,damage)
	if enemy.HP <= 0 or damage <= 0 then
		return false
	end
    enemy.HP = enemy.HP - damage
    if enemy.HP <= 0 then
        enemy.State = enemy.MonsterState.Death
        EnemyEffects.Alive[enemy.ID] = false
        enemy.animation:Play(enemy.animationTabel["death"])
        Group.Remove("MonsterList",enemy.ID)
        local id = Pool.Active("EXP")
        Rigidbody2D.Get(id):SetPosition(enemy.transform.position)
		local experience = Entity.GetScript(id)
		experience:Setting(enemy.EXP or 1)
    else
        enemy.State = enemy.MonsterState.Hit
    end
    return true
end

