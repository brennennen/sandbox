return {
  "nvim-tree/nvim-tree.lua",
  version = "*",
  lazy = false,
  dependencies = {
    "nvim-tree/nvim-web-devicons",
  },
  opts = {
    renderer = {
      icons = {
        show = {
          file = false,
          folder = false,
          folder_arrow = false,
          git = true,
        }
      }
    }
  }
}