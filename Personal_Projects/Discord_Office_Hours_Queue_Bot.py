# Discord bot for handling student queue during office hours
# Can move students in and out of Discord voice channels

# Named Hammond after Hammond, IN


import os
import discord
from discord.utils import get
from discord.ext import commands



# discord stuff

TOKEN = "BOT_TOKEN"


# queue of authors for OH
que = []
que_str = ''
roles = []


#channel references
global oh_waiting_room
global oh_0
global oh_1


# helper functions
def que_to_str(que_in):
    str_out = '\n'  # new lines
    i = 1
    for author in que_in:
        str_out += str(i) + ": " + author.name + "\n"
        i += 1
    return str_out


client = discord.Client()

# Called after initial connection to discord
# Grab references to voice channels
@client.event
async def on_ready():
    global oh_waiting_room
    global oh_0
    global oh_1
    print(f'{client.user} has connected to Discord!')
    for guild in client.guilds:
        if str(guild) == 'SERVER_NAME':
            roles = guild.roles
            for channel in guild.channels:
                if str(channel) == 'VOICE_CHANNEL_NAME_1':
                    oh_waiting_room = channel
                elif str(channel) == 'VOICE_CHANNEL_NAME_2':
                    oh_0 = channel
                elif str(channel) == 'VOICE_CHANNEL_NAME_3':
                    oh_1 = channel


# give everyone student
@client.event
async def on_member_join(member):
    role = get(member.guild.roles, name="ROLE")
    await member.add_roles(role)
    print(f"{member} became a ROLE!")



# the magic
@client.event
async def on_message(message):
    global oh_waiting_room
    global oh_0
    global oh_1
    
    # Only respond to messages in the proper channel, and don't respond to yourself
    if message.author != client.user or str(message.channel) == "hammond-controls" or str(message.channel) == "hammond-control":
        msg = message.content.split(' ')

        # Command prefix
        if msg[0] == '!':
            # display controls
            if msg[1] == 'controls':
                await message.channel.send("""Controls:\n! - denotes a command for Hammond\nadd - add yourself to the office hours queue\nremove - remove yourself from the office hours que\ndisplay - display the current office hours que""")

            # add to que
            if msg[1] == 'add':
                if message.author not in que:
                    que.append(message.author)
                    await message.channel.send(f"added {message.author.name} to the Office Hours que")
                    print(f"added {message.author.name} to the OH que")
            # remove from que
            if msg[1] == 'remove':
                que.remove(message.author)
                print(f"removed {message.author.name} from the OH que")
                    
            # display que
            if msg[1] == 'display':
                que_str = que_to_str(que)
                await message.channel.send(f"que is currently: {que_str}")
 
                
        # Instructor commands - ease of use
        # Allows for quickly moving the current student out of a voice channel,
        # and bringing the next student in que into a voice channel
        elif msg[0] == 'h' and str(message.channel) == "hammond-control":
            # Display controls
            if msg[1] == 'c':
                await message.channel.send("""controls are: 'h ' +\nd - display\nr (i)- remove ith student from the que (h r a to clear the que),\nn - bring the next person in que into your room""")
            # Display que
            if msg[1] == 'd':
                que_str = que_to_str(que)
                await message.channel.send(f"que is currently: {que_str}")
            # remove given student from que
            if msg[1] == 'r':
                if msg[2] == 'a':
                    for i in range(len(que)):
                        del que[i]
                else:
                    x = int(msg[2])-1
                    del_stu = que[x]
                    del que[x]
                    await message.channel.send(f"removed {del_stu.nick} from que")
            # move next student into voice channel - only works for oh_0
            if msg[1] == 'n':
                # noone in the room
                if len(oh_0.members) <= 1:
                    await que[0].move_to(channel=oh_0)
                # move the current student out of the room and remove them from the que
                else:
                    await que[0].move_to(channel=oh_waiting_room)
                    del que[0]
                    await que[0].move_to(channel=oh_0)

            #if msg[1] == '1' and msg[2] == '2':
                
    return
    



client.run(TOKEN)
