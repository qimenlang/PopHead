#include "textWidget.hpp"
#include "gameData.hpp"
#include "Renderer/renderer.hpp"
#include <sstream>

namespace ph {

TextWidget::TextWidget()
	:mTextPosition{0,0}
{
}


void TextWidget::setString(const std::string& text)
{
	//mText = text;
	// TODO: get mSize
}

void TextWidget::setTextPosition(const sf::Vector2f& pos)
{
	//auto localPosition = getGlobalPosition();
	//auto size = getSize();

	//mText.setPosition(pos.x * size.x + localPosition.x, pos.y * size.y + localPosition.y);

	//mTextPosition = pos;
}
//
//void TextWidget::setAlpha(unsigned int alpha)
//{
//	mText.setFillColor(sf::Color(255, 255, 255, alpha));
//	Widget::setAlpha(alpha);
//}

void TextWidget::setTextOrigin(const sf::Vector2f& origin)
{
	//mText.setOrigin(origin);
}

void TextWidget::setTextAlpha(unsigned int alpha)
{
	//mText.setFillColor(sf::Color(255, 255, 255, alpha));
}

void TextWidget::scaleText(const sf::Vector2f& scale)
{
	//mText.scale(scale);
	//setTextPosition(mTextPosition);
}
//
//void TextWidget::setPosition(const sf::Vector2f& pos)
//{
//	Widget::setPosition(pos);
//	setTextPosition(mTextPosition);
//}
//
//void TextWidget::move(const sf::Vector2f& delta)
//{
//	Widget::move(delta);
//	mText.move(delta);
//}

void TextWidget::setCharacterSize(unsigned int size)
{
	//mText.setCharacterSize(size);
}

void TextWidget::setFontPath(const std::string& path)
{
	//mText.setFont(mGameData->getFonts().get(path));
}
//
//void TextWidget::scale(const sf::Vector2f& scale)
//{
//	Widget::scale(scale);
//	scaleText(scale);
//}
//
//void TextWidget::draw()
//{
//	if(mIsActive) {
//		if (scrollingEffect)
//			mText.move(0, -0.35f);
//
//		Widget::draw();
//		Renderer::submitSFMLObject(mText);
//	}
//}

void TextWidget::setScrollingEffect(bool flag)
{
	//scrollingEffect = flag;
}

}
