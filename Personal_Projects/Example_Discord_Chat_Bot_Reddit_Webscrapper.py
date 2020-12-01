# Example of a Discord chat bot hooked up to a Reddit webscrapper
# Responds to requests with posts from subreddits from the requested category


import os
import random
import discord
from discord.utils import get


import praw

### reddit stuff ###
# need to not store this in code
reddit = praw.Reddit(client_id='CLIENT_ID',
                     client_secret='CLIENT_SECRET',
                     user_agent='USER_AGENT',
                     username='USERNAME',
                     password='PASSWORD')

# These are the names of subreddits
subs = ['NATURE_SUB1','NATURE_SUB2','ARCH_SUB1','ARCH_SUB2', 'ARCH_SUB3', \
        'PC_SUB1','PC_SUB2', 'PC_SUB3', 'PC_SUB4']

# Specify the category of each subreddit
nature_subs = ['NATURE_SUB1','NATURE_SUB2']
arch_subs = ['ARCH_SUB1','ARCH_SUB2', 'ARCH_SUB3']
pc_subs = ['PC_SUB1','PC_SUB2', 'PC_SUB3', 'PC_SUB4']

# Contains posts for each sub { 'sub' : [posts] }
posts = {}

# Discord channel vars
roles = []
channels = []
mems = []

# max num of reddit requests
num_posts = 1000

    


    
# grab num_posts posts (or as many posts as you can) from the monthly top of
# each subreddit
# write all links to a file
with open("LINKS_FILENAME", "a") as text_file:
    for sub in subs:
        # web scrapping
        posts[sub] = []
        for post in reddit.subreddit(sub).top('month', limit=num_posts):
            posts[sub].append(post.url)
        # file writing
        text_file.write('\n*** ' + sub + ' ***\n')
        for link in posts[sub]:
            text_file.write(link + '\n')
        print('done with '+str(sub))





### discord stuff ###
global spam

# need to not store this in code
token = "BOT_TOKEN"

client = discord.Client()

# Called after initial connection to discord
@client.event
async def on_ready():
    print(f'{client.user} has connected to Discord!')
    for guild in client.guilds:
        if str(guild) == 'GUILD_NAME':
            gu = guild
            roles = guild.roles
            channels = guild.channels

# Handle new members 
@client.event
async def on_member_join(member):
    role = get(member.guild.roles, name="DEFAULT_ROLE")
    await member.add_roles(role)
    print(f"{member} joined the server!")

    # create private text channel for member
    memd = dict()
    for mem in member.guild.members:
        if (member == mem):
            print(f"{member} can view the new channel")
            memd[mem] = discord.PermissionOverwrite(view_channel=True)
        else:
            print(f"{mem} cannot view the new channel")
            memd[mem] = discord.PermissionOverwrite(view_channel=False)
    await member.guild.create_text_channel(f"{member}'s-private_channel",overwrites=memd)

    
# The Main Event
@client.event
async def on_message(message):
    global spam
    # Don't respond to yourself
    if message.author == client.user:
        return

    # Only respond to msgs that directly reference you
    if message.content[0:3] == 'COMMAND_PREFIX':

        # grab the msg, of form 'COMMAND_PREFIX *# SUB_CATEGORY'
        # *# optional number of posts requested 
        messagetext = message.content.split(' ')

        # spam request - currently only spams on category of subreddit
        if messagetext[1] == 'spam':
            #stop spam
            if len(messagetext) > 2:
                if messagetext[2] == 'stop':
                    spam = False
                    return
            #start spam
            spam=True
            # previous index values that we have already sent
            # prev_x stops repeats within a message
            prev_x = []
            while spam:
                subchoice = random.choice(nature_subs)
                x = random.choice(range(len(posts[subchoice])))
                while x in prev_x:
                    x = random.choice(range(len(posts[subchoice])))
                prev_x.append(x)
                await message.channel.send(posts[subchoice][x])
                
        #check for multi
        if messagetext[1].isnumeric():
            # nature request
            if messagetext[2] == 'nature':
                # prev_x stops repeats within a message
                prev_x = []
                # send the requested nuumber of posts
                for msg in range(int(messagetext[1])):
                    # get a random subreddit from the given category
                    subchoice = random.choice(nature_subs)
                    # get a random post
                    x = random.choice(range(len(posts[subchoice])))
                    # ensure that post was not previously selected
                    while x in prev_x:
                        x = random.choice(range(len(posts[subchoice])))
                    prev_x.append(x)
                    # send post
                    await message.channel.send(posts[subchoice][x])
            # arch request
            if messagetext[2] == 'arch':
                prev_x = []
                for msg in range(int(messagetext[1])):
                    subchoice = random.choice(arch_subs)
                    x = random.choice(range(len(posts[subchoice])))
                    while x in prev_x:
                        x = random.choice(range(len(posts[subchoice])))
                    prev_x.append(x)
                    await message.channel.send(posts[subchoice][x])
            # pc request
            if messagetext[2] == 'pc':
                prev_x = []
                for msg in range(int(messagetext[1])):
                    subchoice = random.choice(pc_subs)
                    x = random.choice(range(len(posts[subchoice])))
                    while x in prev_x:
                        x = random.choice(range(len(posts[subchoice])))
                    prev_x.append(x)
                    await message.channel.send(posts[subchoice][x])

        # singleton
        else:
            # nature request
            if messagetext[1] == 'nature':
                subchoice = random.choice(nature_subs)
                x = random.choice(range(len(posts[subchoice])))
                while x in prev_x:
                    x = random.choice(range(len(posts[subchoice])))
                prev_x.append(x)
                await message.channel.send(posts[subchoice][x])
            # arch request
            if messagetext[1] == 'arch':
                subchoice = random.choice(arch_subs)
                x = random.choice(range(len(posts[subchoice])))
                while x in prev_x:
                    x = random.choice(range(len(posts[subchoice])))
                prev_x.append(x)
                await message.channel.send(posts[subchoice][x])
            # pc request
            if messagetext[1] == 'pc':
                subchoice = random.choice(pc_subs)
                x = random.choice(range(len(posts[subchoice])))
                while x in prev_x:
                    x = random.choice(range(len(posts[subchoice])))
                prev_x.append(x)
                await message.channel.send(posts[subchoice][x])

# start it
client.run(token)
