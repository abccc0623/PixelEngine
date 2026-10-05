ItemType = {
    Stat = 1,
    Factory = 2,
    System = 3,
}

ItemList = 
{
    {
        ItemType = ItemType.Factory,
        Name = "직선 길",
        Texture = "road_1",
        UpgradeContent = "마법책에 자원을 이동시킬 길을 그린다.",
        UpgradeSelectAction = function() InventoryFunction.Add(1,5) end,
        InventoryContent = "Z 키를 눌러 직선 길 설치 합니다",
    },
    {
        ItemType = ItemType.System,
        Name = "생성 시작점",
        Texture = "startPoint",
        UpgradeContent = "자원을 생성하는 룬을 1개 추가합니다",
        UpgradeSelectAction = function() InventoryFunction.Add(3,5) end,
        InventoryContent = "",
    },
    {
        ItemType = ItemType.System,
        Name = "생성 도착점",
        Texture = "endPoint",
        UpgradeContent = "자원을 생성하는 룬을 1개 추가합니다",
        UpgradeSelectAction = function() InventoryFunction.Add(3,5) end,
        InventoryContent = "",
    },
	{
        ItemType = ItemType.Stat,
        Name = "생명력 증가",
        Texture = "endPoint",
        UpgradeContent = "생명력을 1 증가 시킵니다.",
        UpgradeSelectAction = nil,
        InventoryContent = "",
    },
}
