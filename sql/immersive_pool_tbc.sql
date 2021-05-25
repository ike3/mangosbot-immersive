drop table if exists immersive_spawn_fix_m;

create table immersive_spawn_fix_m as
SELECT g.guid,g.spawntimesecs FROM gameobject_template gt 
join gameobject g on gt.entry = g.id
where gt.name in ('Copper Vein','Tin Vein','Silver Vein','Iron Deposit','Gold Vein','Mithril Deposit','Small Thorium Vein','Rich Thorium Vein','Truesilver Vein') 
and g.spawntimesecs in (60, 180);

create index immersive_spawn_fix_m_guid on immersive_spawn_fix_m (guid);

update gameobject set spawntimesecs = spawntimesecs * 15 + 1 -- (spawntimesecs - 1) / 3
where guid in (select guid from immersive_spawn_fix_m);

SELECT distinct g.spawntimesecs, gt.name FROM gameobject_template gt 
join gameobject g on gt.entry = g.id
where gt.name in ('Copper Vein','Tin Vein','Silver Vein','Iron Deposit','Gold Vein','Mithril Deposit','Small Thorium Vein','Rich Thorium Vein','Truesilver Vein') 
-- and g.spawntimesecs in (301, 901)
order by gt.name, g.spawntimesecs desc
;

drop table if exists immersive_spawn_fix_h;

create table immersive_spawn_fix_h as
SELECT g.guid FROM gameobject_template gt 
join gameobject g on gt.entry = g.id
where gt.name in ('Silverleaf','Peacebloom','Earthroot','Mageroyal','Briarthorn','Bruiseweed','Wild Steelbloom','Kingsblood','Grave Moss','Liferoot','Fadeleaf','Khadgar''s Whisker','Wintersbite','Stranglekelp','Goldthorn','Firebloom','Purple Lotus','Arthas'' Tears','Sungrass','Blindweed','Ghost Mushroom','Gromsblood','Golden Sansam','Dreamfoil','Mountain Silversage','Plaguebloom','Icecap','Black Lotus','Bloodthistle','Felweed','Dreaming Glory','Ragveil','Flame Cap','Terocone','Ancient Lichen','Netherbloom','Nightmare Vine','Mana Thistle','Netherdust Bush')
and g.spawntimesecs in (180, 300);

create index immersive_spawn_fix_h_guid on immersive_spawn_fix_h (guid);

update gameobject set spawntimesecs = spawntimesecs * 2 + 1
where guid in (select guid from immersive_spawn_fix_h);

SELECT distinct g.spawntimesecs, gt.name FROM gameobject_template gt 
join gameobject g on gt.entry = g.id
where gt.name in ('Silverleaf','Peacebloom','Earthroot','Mageroyal','Briarthorn','Bruiseweed','Wild Steelbloom','Kingsblood','Grave Moss','Liferoot','Fadeleaf','Khadgar''s Whisker','Wintersbite','Stranglekelp','Goldthorn','Firebloom','Purple Lotus','Arthas'' Tears','Sungrass','Blindweed','Ghost Mushroom','Gromsblood','Golden Sansam','Dreamfoil','Mountain Silversage','Plaguebloom','Icecap','Black Lotus','Bloodthistle','Felweed','Dreaming Glory','Ragveil','Flame Cap','Terocone','Ancient Lichen','Netherbloom','Nightmare Vine','Mana Thistle','Netherdust Bush')
order by gt.name, g.spawntimesecs desc
;

select * from immersive_spawn_fix;