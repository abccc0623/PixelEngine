-- Grid installation must work without Graph or EnergyFunction globals.
Grid = {width = 1, height = 1, StartPoint = {x = 0,y = 0}, EndPoint = {x = 0,y = 0}, Visible = true}
Grid[1] = {{ItemIndex = 0,ItemRotation = 0}}
ItemType = {Factory = 2}
ItemList = {{ItemType = ItemType.Factory,Texture = "road_3"}}
local drawn, refunded = 0,0
local image = {thisID = 1}
function image:SetOrder(value)
end
function image:SetColor(...)
end
function image:SetPivot(...)
end
Grid.UI = {{image}}
Grid.Transform = {{{rotation = {z = 0}}}}
ItemVisual = {}
function ItemVisual.Apply(target,item,width,height,unscaled,texture)
    drawn = drawn + 1
    assert(texture == (item and item.Texture or "tile"))
end
Entity = {}
function Entity.SetActive(id,active)
    assert(id == 1)
end
InventoryFunction = {}
function InventoryFunction.Add(index,count)
    refunded = refunded + count
    return true
end
dofile("Asset/Script/Grid/GridFunction.lua")
assert(Graph == nil and EnergyFunction == nil)
assert(GridFunction.Add(1,1,1))
assert(GridFunction.Get(1,1) == 1)
assert(GridFunction.Remove(1,1))
assert(GridFunction.Get(1,1) == 0 and refunded == 1 and drawn == 2)
print("grid_without_graph: PASS")
