-- Default
vim.opt.number = true
vim.opt.termguicolors = true
vim.opt.clipboard = 'unnamedplus'
vim.opt.expandtab = true
vim.opt.shiftwidth = 4
vim.opt.tabstop = 4
vim.opt.softtabstop = 4
vim.opt.autoindent = true
vim.opt.smartindent = true
vim.g.loaded_netrw = 1
vim.g.loaded_netrwPlugin = 1

-- Clipboard over ssh
vim.g.clipboard = {
  name = 'OSC 52',
  copy = {
    ['+'] = require('vim.ui.clipboard.osc52').copy('+'),
    ['*'] = require('vim.ui.clipboard.osc52').copy('*'),
  },
  paste = {
    ['+'] = require('vim.ui.clipboard.osc52').paste('+'),
    ['*'] = require('vim.ui.clipboard.osc52').paste('*'),
  },
}

-- Trailing whitespace
local trim_whitespace_augroup = vim.api.nvim_create_augroup("TrimWhitespace", { clear = true })
vim.api.nvim_create_autocmd("BufWritePre", {
  group = trim_whitespace_augroup,
  pattern = "*",
  callback = function()
    if vim.b.editorconfig and vim.b.editorconfig.trim_trailing_whitespace then
      local view = vim.fn.winsaveview()
      vim.cmd("keeppatterns %s/\\s\\+$//e")
      vim.fn.winrestview(view)
    end
  end,
})

-- File type specific
local file_type_augroup = vim.api.nvim_create_augroup("FileTypeSettings", { clear = true })
-- Makefiles
vim.api.nvim_create_autocmd("FileType", {
  pattern = "make",
  group = file_type_augroup,
  callback = function()
    vim.bo.expandtab = false
    vim.bo.shiftwidth = 8
    vim.bo.tabstop = 8
    vim.bo.softtabstop = 8
  end,
})

-- Package managers
require("config.lazy")

-- nvim-tree
vim.api.nvim_create_autocmd("VimEnter", {
  callback = function()
    if not vim.opt.diff:get() and vim.fn.argc() > 0 then
      vim.cmd.NvimTreeOpen()
      vim.cmd.wincmd("p")
    end
  end,
})
