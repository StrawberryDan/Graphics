#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------


//======================================================================================================================
//  Class Declaration
//----------------------------------------------------------------------------------------------------------------------
#include "Transform2D.hpp"
// Strawberry Core
#include "Strawberry/Core/Types/ReflexivePointer.hpp"


namespace Strawberry::Graphics
{
	class SpriteSheet;


	class Sprite
	{
		friend class SpriteRenderer;


	public:
		Sprite(const SpriteSheet& spriteSheet);

		[[nodiscard]] const Core::ReflexivePointer<SpriteSheet>& GetSpriteSheet() const;
		void SetSpriteSheet(const SpriteSheet& spriteSheet);

		[[nodiscard]] const Core::Math::Vec2u& GetSpriteCoords() const;
		void SetSpriteCoords(const Core::Math::Vec2u& spriteCoords);

		uint32_t GetSpriteIndex() const;
		void SetSpriteIndex(uint32_t index);


	private:
		Core::ReflexivePointer<SpriteSheet> mSpriteSheet;
		Core::Math::Vec2u mSpriteCoords;
	};
} // Graphics
