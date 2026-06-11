.PHONY: dev pangu

dev:
	npx quartz build --serve

# 中英文间距（pangu 风格），直接写回 content/
pangu:
	npx tsx scripts/pangu.ts

update:
	npx quartz plugin install --latest