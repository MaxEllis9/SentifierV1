#pragma once

#include <JuceHeader.h>

namespace Gui
{
    class Bulb : public Component
    {
    public:
        Bulb(const Colour& c) : colour(c){}
        
        void paint(Graphics& g) override
        {
            const auto delta = 4.f;
            const auto bounds = getLocalBounds().toFloat().reduced(delta);
            const auto side = jmin(bounds.getHeight(), bounds.getWidth());
            const auto bulbFillBounds = Rectangle<float>(bounds.getX(), bounds.getY(), side, side);
            if(isOn){
                g.setColour(colour);
            }
            else
            {
                g.setColour(Colours::black);
            }
            g.fillEllipse(bulbFillBounds);
            g.setColour(Colours::black);
            g.drawEllipse(bulbFillBounds, 1.f);
            if(isOn)
            {
                g.setGradientFill(
                  ColourGradient{
                      colour.withAlpha(0.3f),
                      bulbFillBounds.getCentre(),
                      colour.withLightness(1.5f).withAlpha(0.f),
                      {},
                      true
                  });
                g.fillEllipse(bulbFillBounds.expanded(delta));
            }
        }
        
        void setState(const bool state)
        {
            isOn = state;
            repaint();
        }
        
    private:
        bool isOn = false;
        Colour colour{};
    };


    class GainMeter : public Component, public Timer
    {
    public:
        GainMeter(std::function<float()>&& valueFunction) : valueSupplier(std::move(valueFunction))
        {
            startTimerHz(24);
        }
        
        void paint(Graphics& g) override
        {
            if(!toggleLights)
            {
                for(auto i=0; i<totalBulbNo; i++){
                    bulbs[i]->setState(false);
                }
            }
            else
            {
                const auto level = jmap(valueSupplier(), -60.f, 6.f, 0.f, 1.f);
                for(auto i=0; i<totalBulbNo; i++){
                    if(level >= static_cast<float>(i+1)/totalBulbNo){
                        bulbs[i]->setState(true);
                    }
                    else{
                        bulbs[i]->setState(false);
                    }
                }
            }
            
        }
        
        void resized() override
        {
            const auto bounds = getLocalBounds().toFloat();
            gradient = ColourGradient{
                Colours::green,
                bounds.getBottomLeft(),
                Colours::red,
                bounds.getTopLeft(),
                false
            };
            gradient.addColour(0.5, Colours::yellow);
            
            auto bulbBounds = getLocalBounds();
            const auto bulbHeight = bulbBounds.getHeight() / totalBulbNo;
            bulbs.clear();
            
            for(auto i = 0; i<totalBulbNo; i++){
                auto bulb = std::make_unique<Bulb>(gradient.getColourAtPosition(static_cast<double>(i) / totalBulbNo));
                addAndMakeVisible(bulb.get());
                bulb->setBounds(bulbBounds.removeFromBottom(bulbHeight));
                bulbs.push_back(std::move(bulb));
            }
            
        }
        
        void timerCallback() override
        {
            repaint();
        }
        
        void toggleMeterEnablement(bool enabled)
        {
            toggleLights = enabled;
        }
        
    private:
        std::function<float()> valueSupplier;
        std::vector<std::unique_ptr<Bulb>> bulbs;
        ColourGradient gradient;
        const int totalBulbNo = 10;
        bool toggleLights = true;
    };
}
