-- Default Lua Script
InventoryType = {
    Tool = 1,
    Inventory = 2,
}

User = {
    Selection = {Type = InventoryType.Tool, Index = 1},
    SelectTransform = nil,
    InventoryDescriptionID = nil,
    ToolsComponent = {},
    InventoryComponent = {},

    Tools = {
        {
            Name = "삭제",
            Content = "z키를 눌러 해당 칸의 데이터를 삭제",
            Texture = "delete",
            Action = function(x,y) GridFunction.Remove(x,y) end
        },
        {
            Name = "회전",
            Content = "z키를 눌러 해당 칸의 데이터를 삭제",
            Texture = "rotation",
            Action = function(x,y) GridFunction.Rotation(x,y) end
        },
    },
    
    Inventory = 
    {
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
          {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
        {
            ItemIndex = 0,
            ItemCount = 0,
        },
    },
}
