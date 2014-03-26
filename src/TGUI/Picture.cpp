/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TGUI - Texus's Graphical User Interface
// Copyright (C) 2012-2014 Bruno Van de Velde (vdv_b@tgui.eu)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <TGUI/Picture.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Picture::Picture()
    {
        m_callback.widgetType = Type_Picture;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Picture::Picture(const Picture& copy) :
    ClickableWidget (copy),
    m_loadedFilename(copy.m_loadedFilename)
    {
        // Copy the texture
        TGUI_TextureManager.copyTexture(copy.m_texture, m_texture);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Picture::~Picture()
    {
        // Remove the texture (if we are the only one using it)
        if (m_texture.getData() != nullptr)
            TGUI_TextureManager.removeTexture(m_texture);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Picture& Picture::operator= (const Picture& right)
    {
        // Make sure it is not the same widget
        if (this != &right)
        {
            Picture temp(right);
            this->ClickableWidget::operator=(right);

            std::swap(m_loadedFilename, temp.m_loadedFilename);
            std::swap(m_texture,        temp.m_texture);
        }

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Picture* Picture::clone()
    {
        return new Picture(*this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool Picture::load(const std::string& filename)
    {
        // When everything is loaded successfully, this will become true.
        m_loaded = false;
        m_size.x = 0;
        m_size.y = 0;

        // Make sure that the filename isn't empty
        if (filename.empty())
            return false;

        m_loadedFilename = getResourcePath() + filename;

        // If we have already loaded a texture then first delete it
        if (m_texture.getData() != nullptr)
            TGUI_TextureManager.removeTexture(m_texture);

        // Try to load the texture from the file
        if (TGUI_TextureManager.getTexture(m_texture, m_loadedFilename))
        {
            m_loaded = true;

            // Remember the size of the texture
            setSize(static_cast<float>(m_texture.getSize().x),static_cast<float>( m_texture.getSize().y));

            return true;
        }
        else // The texture was not loaded
            return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const std::string& Picture::getLoadedFilename() const
    {
        return m_loadedFilename;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Picture::setPosition(float x, float y)
    {
        Transformable::setPosition(x, y);

        m_texture.setPosition(x, y);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Picture::setSize(float width, float height)
    {
        m_size.x = width;
        m_size.y = height;

        if (m_loaded)
            m_texture.setSize(width, height);
        else
            TGUI_OUTPUT("TGUI warning: Picture::setSize called while Picture wasn't loaded yet.");
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Picture::setSmooth(bool smooth)
    {
        if (m_loaded)
            m_texture.getData()->texture.setSmooth(smooth);
        else
            TGUI_OUTPUT("TGUI warning: Picture::setSmooth called while Picture wasn't loaded yet.");
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool Picture::isSmooth() const
    {
        if (m_loaded)
            return m_texture.getData()->texture.isSmooth();
        else
        {
            TGUI_OUTPUT("TGUI warning: Picture::isSmooth called while Picture wasn't loaded yet.");
            return false;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Picture::setTransparency(unsigned char transparency)
    {
        ClickableWidget::setTransparency(transparency);

        m_texture.setColor(sf::Color(255, 255, 255, m_opacity));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool Picture::mouseOnWidget(float x, float y)
    {
        // Don't do anything when the image wasn't loaded
        if (m_loaded == false)
            return false;

        // Check if the mouse is on top of the picture
        if (getTransform().transformRect(sf::FloatRect(0, 0, m_size.x, m_size.y)).contains(x, y))
        {
            sf::Vector2f scaling;
            scaling.x = m_size.x / m_texture.getSize().x;
            scaling.y = m_size.y / m_texture.getSize().y;

            // Only return true when the pixel under the mouse isn't transparent
            if (!m_texture.isTransparentPixel(static_cast<unsigned int>((x - getPosition().x) / scaling.x), static_cast<unsigned int>((y - getPosition().y) / scaling.y)))
                return true;
        }

        if (m_mouseHover == true)
            mouseLeftWidget();

        m_mouseHover = false;
        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool Picture::setProperty(std::string property, const std::string& value)
    {
        property = toLower(property);

        if (property == "filename")
        {
            load(value);
        }
        else if (property == "smooth")
        {
            if ((value == "true") || (value == "True"))
                setSmooth(true);
            else if ((value == "false") || (value == "False"))
                setSmooth(false);
            else
                TGUI_OUTPUT("TGUI error: Failed to parse 'Smooth' property.");
        }
        else // The property didn't match
            return ClickableWidget::setProperty(property, value);

        // You pass here when one of the properties matched
        return true;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool Picture::getProperty(std::string property, std::string& value) const
    {
        property = toLower(property);

        if (property == "filename")
            value = getLoadedFilename();
        else if (property == "smooth")
            value = isSmooth() ? "true" : "false";
        else // The property didn't match
            return ClickableWidget::getProperty(property, value);

        // You pass here when one of the properties matched
        return true;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::list< std::pair<std::string, std::string> > Picture::getPropertyList() const
    {
        auto list = ClickableWidget::getPropertyList();
        list.push_back(std::pair<std::string, std::string>("Filename", "string"));
        list.push_back(std::pair<std::string, std::string>("Smooth", "bool"));
        return list;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Picture::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_texture, states);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
