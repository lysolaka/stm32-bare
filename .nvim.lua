-- set `vim.o["exrc"] = true` and `:trust` this file in order to enable this configuration
vim.lsp.config("clangd", {
  cmd = { "clangd", "--query-driver", "/usr/bin/arm-none-eabi-*", "--clang-tidy" }
})

vim.lsp.config("asm_lsp", {
  filetypes = { "asm", "gas" },
})
