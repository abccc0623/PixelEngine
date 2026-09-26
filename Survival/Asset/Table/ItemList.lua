ItemList = 
{
    {
        Name = "직선 길",
        IsConveyor = true,
        Texture = "line",
        UpgradeContent = "마법책에 자원을 이동시킬 길을 그린다.",
        UpgradeAction = function() InventoryFunction.Add(1,5) end,
        InventoryContent = "Z 키를 눌러 직선 길 설치 합니다",
    },
    {
        Name = "생성 시작점",
        Texture = "startPoint",
        UpgradeContent = "자원을 생성하는 룬을 1개 추가합니다",
        UpgradeAction = function() InventoryFunction.Add(3,5) end,
        InventoryContent = "Z 키를 눌러 설치 합니다",
    },
    {
        Name = "3",
        Texture = "line",
        UpgradeContent = "인벤토리에 설치가 가능한 직선 통로를 5개 추가한다",
        UpgradeAction = function() InventoryFunction.Add(4,5) end,
        InventoryContent = "Z 키를 눌러 직선 통로를 설치 합니다",
    },
    {
        Name = "4",
        Texture = "line",
        UpgradeContent = "인벤토리에 설치가 가능한 직선 통로를 5개 추가한다",
        UpgradeAction = function() InventoryFunction.Add(4,5) end,
        InventoryContent = "Z 키를 눌러 직선 통로를 설치 합니다",
    },
}
