/* *****************************************************************************
 * The method lives() is based on Xitari's code, from Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 * *****************************************************************************
 * A.L.E (Arcade Learning Environment)
 * Copyright (c) 2009-2013 by Yavar Naddaf, Joel Veness, Marc G. Bellemare and 
 *   the Reinforcement Learning and Artificial Intelligence Laboratory
 * Released under the GNU General Public License; see License.txt for details. 
 *
 * Based on: Stella  --  "An Atari 2600 VCS Emulator"
 * Copyright (c) 1995-2007 by Bradford W. Mott and the Stella team
 *
 * *****************************************************************************
 */
#include "SpaceInvaders.hpp"
#include <iostream>

#include "../RomUtils.hpp"

ActionVect SpaceInvadersSettings::actions;

SpaceInvadersSettings::SpaceInvadersSettings() {
    reset();
}


/* create a new instance of the rom */
RomSettings* SpaceInvadersSettings::clone() const { 
    
    RomSettings* rval = new SpaceInvadersSettings();
    *rval = *this;
    return rval;
}


/* process the latest information from ALE */
void SpaceInvadersSettings::step(const System& system) {

    // update the reward
    int score_1 = getDecimalScore(0xE8, 0xE6, &system);
    // by trial and error, found the location of player 2's score
    int score_2 = getDecimalScore(0xE9, 0xE7, &system);
    // reward cannot get negative in this game. When it does, it means that the score has looped 
    // (overflow)
    m_reward_1 = score_1 - m_score_1;
    m_reward_2 = score_2 - m_score_2;
    if(m_reward_1 < 0) {
        // 10000 is the highest possible score
        const int maximumScore = 10000;
        m_reward_1 = (maximumScore - m_score_1) + score_1;
    }
    if(m_reward_2 < 0) {
        // 10000 is the highest possible score
        const int maximumScore = 10000;
        m_reward_2 = (maximumScore - m_score_2) + score_2;
    }
    m_score_1 = score_1;
    m_score_2 = score_2;
    m_lives = readRam(&system, 0xC9);

    // update terminal status
    // If bit 0x80 is on, then game is over 
    int some_byte = readRam(&system, 0x98); 
    m_terminal = (some_byte & 0x80) || m_lives == 0;
}


/* is end of game */
bool SpaceInvadersSettings::isTerminal() const {

    return m_terminal;
};


/* get the most recently observed reward */
std::pair<reward_t, reward_t> SpaceInvadersSettings::getReward() const { 
    return std::make_pair(m_reward_1,m_reward_2); 
}


/* is an action part of the minimal set? */
bool SpaceInvadersSettings::isMinimal(const Action &a) const {

    switch (a) {
        case PLAYER_A_NOOP:
        case PLAYER_A_LEFT:
        case PLAYER_A_RIGHT:
        case PLAYER_A_FIRE:
        case PLAYER_A_LEFTFIRE:
        case PLAYER_A_RIGHTFIRE:
            return true;
        default:
            return false;
    }   
}


/* reset the state of the game */
void SpaceInvadersSettings::reset() {
    
    m_reward_1 = 0;
    m_score_1  = 0;
    m_reward_2 = 0;
    m_score_2  = 0;
    m_terminal = false;
    m_lives    = 3; 
}

        
/* saves the state of the rom settings */
void SpaceInvadersSettings::saveState(Serializer & ser) {
  ser.putInt(m_reward_1);
  ser.putInt(m_score_1);
  ser.putInt(m_reward_2);
  ser.putInt(m_score_2);
  ser.putBool(m_terminal);
  ser.putInt(m_lives);
}

// loads the state of the rom settings
void SpaceInvadersSettings::loadState(Deserializer & ser) {
  m_reward_1 = ser.getInt();
  m_score_1 = ser.getInt();
  m_reward_2 = ser.getInt();
  m_score_2 = ser.getInt();
  m_terminal = ser.getBool();
  m_lives = ser.getInt();
}

// returns a list of mode that the game can be played in
ModeVect SpaceInvadersSettings::getAvailableModes() {
    ModeVect modes(getNumModes());
    for (unsigned int i = 0; i < modes.size(); i++) {
        modes[i] = i;
    }
    return modes;
}

// set the mode of the game
// the given mode must be one returned by the previous function
void SpaceInvadersSettings::setMode(game_mode_t m, System &system,
                              std::unique_ptr<StellaEnvironmentWrapper> environment) {

    if(m < getNumModes()) {
        // read the mode we are currently in
        unsigned char mode = readRam(&system, 0xDC);
        // press select until the correct mode is reached
        while (mode != m) {
            environment->pressSelect(2);
            mode = readRam(&system, 0xDC);
        }
        //reset the environment to apply changes.
        environment->softReset();
    }
    else {
        throw std::runtime_error("This mode doesn't currently exist for this game");
    }
 }

DifficultyVect SpaceInvadersSettings::getAvailableDifficulties() {
    DifficultyVect diff = {0, 1};
    return diff;
}
