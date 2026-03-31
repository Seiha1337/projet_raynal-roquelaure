-- ==========================================================
-- 🗄️ BASE DE DONNÉES UNIFIÉE : RAYNAL & ROQUELAURE
-- ==========================================================

-- 1. Création de la base de données
CREATE DATABASE IF NOT EXISTS raynal_roquelaure_prod;
USE raynal_roquelaure_prod;

-- 2. Structure de la table unifiée
CREATE TABLE `mesures_autoclaves` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `autoclave_id` int(11) NOT NULL DEFAULT 1,
  `date_heure` timestamp NULL DEFAULT current_timestamp(),
  `temperature` float NOT NULL,
  `consigne` float DEFAULT NULL,
  `pression` float DEFAULT NULL,
  `cycle` varchar(50) DEFAULT NULL,
  `etat` varchar(20) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;