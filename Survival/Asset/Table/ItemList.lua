 ItemType = {
    Stat = 1,
    Factory = 2,
    System = 3,
}

FactoryType = {
    Distribution = 1, -- 자원의 경로·개수 변경
    Converter = 2, -- 자원에 추가 효과 부여
    Generator = 3, -- 독립적으로 자원 생성 (기능 미구현)
}

ItemList = 
{
    {
        ItemType = ItemType.Factory,
        Name = "직선 길",
        FactoryType = FactoryType.Distribution,
        Texture = "road_1",
        Input = {x = -1,y = 0},
        OutPut = {x = 1,y = 0},
        UpgradeContent = "마법책에 자원을 이동시킬 길을 그린다.",
        UpgradeSelectAction = function() InventoryFunction.Add(1,20) end,
        InventoryContent = "Z 키를 눌러 직선 길 설치 합니다",
    },
    {
        ItemType = ItemType.System,
        Name = "생성 시작점",
        Texture = "item_01",
        OutPut = {x = 1,y = 0},
        UpgradeContent = "자원을 생성하는 룬을 1개 추가합니다",
        UpgradeSelectAction = function() InventoryFunction.Add(3,5) end,
        InventoryContent = "",
    },
    {
        ItemType = ItemType.System,
        Name = "생성 도착점",
        Texture = "item_00",
        Input = {x = -1,y = 0},
        UpgradeContent = "자원을 생성하는 룬을 1개 추가합니다",
        UpgradeSelectAction = function() InventoryFunction.Add(3,5) end,
        InventoryContent = "",
    },
}
