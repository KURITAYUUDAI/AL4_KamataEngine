#include "BitmapFont.h"
#include <fstream>
#include <sstream>

BitmapFont& BitmapFont::GetInstance() 
{
	static BitmapFont instance;
	return instance;
}

bool BitmapFont::Load(const std::string& fntPath, const std::string& textureName) 
{
	textureHandle_ = TextureManager::Load(textureName);
	if (textureHandle_ == 0) 
	{
		return false;
	}

	std::ifstream ifs(fntPath);
	if (!ifs.is_open())
		return false;

	std::string line;
	while (std::getline(ifs, line)) 
	{
		if (line.find("char id") != std::string::npos) 
		{
			std::istringstream iss(line);
			std::string token;
			Glyph glyph{};
			char id = 0;

			while (iss >> token) 
			{
				auto pos = token.find('=');
				if (pos == std::string::npos)
					continue;
				std::string key = token.substr(0, pos);
				std::string value = token.substr(pos + 1);

				if (key == "id")
					id = static_cast<char>(std::stoi(value));
				else if (key == "x")
					glyph.texBase.x = std::stof(value);
				else if (key == "y")
					glyph.texBase.y = std::stof(value);
				else if (key == "width")
					glyph.texSize.x = std::stof(value);
				else if (key == "height")
					glyph.texSize.y = std::stof(value);
				else if (key == "xoffset")
					glyph.offset.x = std::stof(value);
				else if (key == "yoffset")
					glyph.offset.y = std::stof(value);
				else if (key == "xadvance")
					glyph.xAdvance = std::stof(value);
			}
			glyphs_[id] = glyph;
		}
	}
	return true;
}

void BitmapFont::DrawText(const std::string& text, const Vector2& position, float size) 
{
	Vector2 cursor = position;
	for (char c : text) {
		auto it = glyphs_.find(c);
		if (it == glyphs_.end())
			continue;

		const Glyph& glyph = it->second;

		Sprite* sprite = Sprite::Create(textureHandle_, cursor + glyph.offset * size);
		sprite->SetTextureRect(glyph.texBase, glyph.texSize);
		sprite->SetSize({size, size});
		sprite->Draw();

		cursor.x += glyph.xAdvance * size;
	}
}
